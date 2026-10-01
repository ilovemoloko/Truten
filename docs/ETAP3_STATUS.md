# ETAP 3 status

- Issue #3 `[client]: authorization`: implemented refresh-token flow in client. On a protected request returning HTTP 401, the client calls `/auth/refresh`, stores the new access token, and retries the original request exactly once. A second 401 clears the local session and is surfaced as an authentication error.
- Issue #4 `[server] db with handlers`: PASS from ETAP 3 audit.
- Issue #5 `[server] tests`: PASS from ETAP 2/3 audit.
- Issue #6 `[server] admin handlers`: PASS from ETAP 1/3 audit.
- Issue #7 `[server] slot statuses`: DEFERRED / BLOCKED BY PRODUCT SPEC. The issue has no body and the repository does not define the required status model. Do not invent statuses. Product specification is required before implementation.
- `TODO: add campus selection`: BACKLOG.
- `TODO: not mvp` for `/stats`: BACKLOG.
