#!/usr/bin/env python3
import json
import os
import subprocess
import sys
import time
import urllib.error
import urllib.request
from concurrent.futures import ThreadPoolExecutor

PORT = os.environ.get("TRUTEN_CONCURRENCY_PORT", "8082")
BASE = f"http://127.0.0.1:{PORT}"
DB_HOST = os.environ.get("TRUTEN_TEST_DB_HOST", "127.0.0.1")
DB_PORT = os.environ.get("TRUTEN_TEST_DB_PORT", "5432")
DB_NAME = os.environ.get("TRUTEN_TEST_DB_NAME", "truten_test")
DB_USER = os.environ.get("TRUTEN_TEST_DB_USER", "lavrinajtisaleksei")
DB_PASS = os.environ.get("TRUTEN_TEST_DB_PASS", "")


def fail(message):
    print(f"FAIL: {message}", file=sys.stderr)
    raise SystemExit(1)


def request(method, path, body=None, token=None, timeout=8):
    data = None if body is None else json.dumps(body).encode()
    headers = {"Content-Type": "application/json"}
    if token:
        headers["Authorization"] = f"Bearer {token}"
    req = urllib.request.Request(BASE + path, data=data, headers=headers, method=method)
    try:
        with urllib.request.urlopen(req, timeout=timeout) as resp:
            raw = resp.read().decode()
            return resp.status, json.loads(raw) if raw else {}
    except urllib.error.HTTPError as exc:
        raw = exc.read().decode()
        try:
            payload = json.loads(raw) if raw else {}
        except json.JSONDecodeError:
            payload = {"raw": raw}
        return exc.code, payload


def wait_for_server(timeout=12):
    deadline = time.time() + timeout
    while time.time() < deadline:
        try:
            status, _ = request("GET", "/", timeout=1)
            if status:
                return
        except Exception:
            time.sleep(0.2)
    fail("server did not start")


def psql(sql):
    env = os.environ.copy()
    if DB_PASS:
        env["PGPASSWORD"] = DB_PASS
    cmd = ["psql", "-h", DB_HOST, "-p", DB_PORT, "-U", DB_USER, "-d", DB_NAME,
           "-v", "ON_ERROR_STOP=1", "-c", sql]
    subprocess.run(cmd, check=True, env=env, stdout=subprocess.DEVNULL)


def register(email, password, name):
    status, payload = request("POST", "/v1/auth/createAccount",
                              {"email": email, "password": password, "name": name})
    if status != 200:
        fail(f"register {email}: HTTP {status} {payload}")
    return payload["userId"], payload["accessToken"]


def login(email, password):
    status, payload = request("POST", "/v1/auth/login", {"email": email, "password": password})
    if status != 200:
        fail(f"login {email}: HTTP {status} {payload}")
    return payload["accessToken"]


def create_slot(admin_token, gym_id, start_time, end_time, capacity):
    status, before = request("GET", f"/v1/sections/{gym_id}/slots", token=admin_token)
    if status != 200:
        fail(f"get slots before create: HTTP {status} {before}")
    before_ids = {s.get("slotId") for s in before.get("slots", [])}

    status, payload = request("POST", "/v1/slots", {
        "gymId": gym_id,
        "startTime": start_time,
        "endTime": end_time,
        "capacity": capacity,
    }, admin_token)
    if status != 200:
        fail(f"create slot: HTTP {status} {payload}")

    status, after = request("GET", f"/v1/sections/{gym_id}/slots", token=admin_token)
    if status != 200:
        fail(f"get slots after create: HTTP {status} {after}")
    created = [s for s in after.get("slots", []) if s.get("slotId") not in before_ids]
    if len(created) != 1:
        fail(f"expected one newly created slot, got {len(created)}")
    return created[0]["slotId"]


def parallel_calls(calls):
    with ThreadPoolExecutor(max_workers=len(calls)) as pool:
        futures = [pool.submit(request, *call) for call in calls]
        return [f.result(timeout=10) for f in futures]


def assert_codes(results, expected):
    codes = sorted(status for status, _ in results)
    if codes != sorted(expected):
        fail(f"unexpected HTTP codes: got {codes}, expected {sorted(expected)}")


def main():
    if len(sys.argv) != 2:
        fail("usage: concurrency_test.py /path/to/Server")

    server_bin = sys.argv[1]
    env = os.environ.copy()
    env.update({
        "DB_HOST": DB_HOST,
        "DB_PORT": DB_PORT,
        "DB_NAME": DB_NAME,
        "DB_USER": DB_USER,
        "DB_PASS": DB_PASS,
        "JWT_SECRET": "truten-concurrency-test-secret",
        "PORT": PORT,
    })

    server = subprocess.Popen([server_bin], env=env, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True)
    try:
        wait_for_server()

        admin_id, _ = register("concurrency-admin@test.local", "adminpass", "Concurrency Admin")
        psql("UPDATE users SET is_admin = TRUE WHERE email = 'concurrency-admin@test.local';")
        admin_token = login("concurrency-admin@test.local", "adminpass")

        status, payload = request("POST", "/v1/sections/gymList",
                                  {"gymName": "Concurrency Gym", "creatorId": admin_id}, admin_token)
        if status != 200:
            fail(f"create gym: HTTP {status} {payload}")
        status, payload = request("GET", "/v1/sections/gymList", token=admin_token)
        gyms = [g for g in payload.get("sections", []) if g.get("name") == "Concurrency Gym"]
        if not gyms:
            fail("concurrency gym not found")
        gym_id = gyms[-1]["id"]

        # 2.3: two users race for one remaining place.
        race1_id, race1_token = register("race1@test.local", "race1pass", "Race One")
        race2_id, race2_token = register("race2@test.local", "race2pass", "Race Two")
        race_slot = create_slot(admin_token, gym_id, "2031-01-01T10:00:00", "2031-01-01T11:00:00", 1)
        results = parallel_calls([
            ("POST", f"/v1/slots/{race_slot}/entries", {"userId": race1_id}, race1_token),
            ("POST", f"/v1/slots/{race_slot}/entries", {"userId": race2_id}, race2_token),
        ])
        assert_codes(results, [200, 400])
        status, slot = request("GET", f"/v1/slots/{race_slot}", token=admin_token)
        if status != 200 or slot.get("participantsCount") != 1:
            fail(f"last-slot race violated capacity: HTTP {status} {slot}")
        print("PASS 2.3 concurrent last-slot booking: exactly one winner")

        # 2.4: duplicate concurrent cancellation must not auto-promote twice.
        q1_id, q1_token = register("queue1@test.local", "queue1pass", "Queue One")
        q2_id, q2_token = register("queue2@test.local", "queue2pass", "Queue Two")
        queue_slot = create_slot(admin_token, gym_id, "2031-01-02T10:00:00", "2031-01-02T11:00:00", 1)
        status, _ = request("POST", f"/v1/slots/{queue_slot}/entries", {"userId": admin_id}, admin_token)
        if status != 200:
            fail(f"seed enrollment failed: HTTP {status}")
        for uid, tok in ((q1_id, q1_token), (q2_id, q2_token)):
            status, payload = request("POST", f"/v1/queue/{queue_slot}/join", {"userId": uid}, tok)
            if status != 200 or not payload.get("queued"):
                fail(f"queue seed failed for {uid}: HTTP {status} {payload}")

        results = parallel_calls([
            ("DELETE", f"/v1/slots/{queue_slot}/entries", {"userId": admin_id}, admin_token),
            ("DELETE", f"/v1/slots/{queue_slot}/entries", {"userId": admin_id}, admin_token),
        ])
        assert_codes(results, [204, 404])

        status, slot = request("GET", f"/v1/slots/{queue_slot}", token=admin_token)
        if status != 200:
            fail(f"read queue slot failed: HTTP {status} {slot}")
        participants = [x.get("userId") for x in slot.get("participants", [])]
        queued = [x.get("userId") for x in slot.get("queue", [])]
        candidates = {q1_id, q2_id}
        if len(participants) != 1 or len(queued) != 1:
            fail(f"promotion cardinality wrong: participants={participants}, queue={queued}")
        if set(participants + queued) != candidates:
            fail(f"lost/duplicated queued users: participants={participants}, queue={queued}")
        if len(set(participants)) != len(participants) or len(set(queued)) != len(queued):
            fail("duplicate user detected after concurrent cancellation")
        print("PASS 2.4 concurrent queue promotion: no duplicate/lost/double promotion")

        print("CONCURRENCY_TESTS=PASS")
    finally:
        server.terminate()
        try:
            server.wait(timeout=3)
        except subprocess.TimeoutExpired:
            server.kill()
        try:
            psql("DROP TABLE IF EXISTS users, slots, gyms;")
        except Exception:
            pass


if __name__ == "__main__":
    main()
