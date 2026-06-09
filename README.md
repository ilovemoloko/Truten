# Truten — система записи в спортивные залы

Десктопное приложение (Qt/QML) + C++ сервер (Crow) + PostgreSQL.

---

## Требования

- Ubuntu 22.04
- Git

---

## 1. Зависимости

```bash
sudo apt update
sudo apt install -y \
    git cmake g++ pkg-config \
    libssl-dev \
    libpqxx-dev \
    postgresql postgresql-client \
    qt6-base-dev qt6-declarative-dev \
    qml6-module-qtquick qml6-module-qtquick-controls \
    qml6-module-qtquick-layouts
```

---

## 2. База данных

```bash
# Запустить PostgreSQL
sudo systemctl start postgresql
sudo systemctl enable postgresql

# Создать пользователя и базу данных
sudo -u postgres psql <<'EOF'
CREATE USER truten_user WITH PASSWORD 'yourpassword';
CREATE DATABASE truten OWNER truten_user;
GRANT ALL PRIVILEGES ON DATABASE truten TO truten_user;
EOF
```

---

## 3. Клонирование

```bash
git clone https://github.com/your-repo/Truten.git
cd Truten
```

---

## 4. Настройка окружения

Создайте файл `.env` рядом с корневым `README.md`:

```
DB_HOST=localhost
DB_PORT=5432
DB_NAME=truten
DB_USER=truten_user
DB_PASS=yourpassword
JWT_SECRET=замените_на_случайную_строку
```

> Таблицы в БД создаются автоматически при первом запуске сервера.

---

## 5. Запуск сервера

```bash
mkdir -p server/build && cd server/build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --parallel $(nproc)
./Server
```

Сервер запустится на `http://0.0.0.0:8080`. Оставьте терминал открытым.

---

## 6. Запуск клиента

Выполните одну команду в терминале Ubuntu:

```bash
curl -fsSL https://raw.githubusercontent.com/ilovemoloko/Truten/clientStuff/install.sh | bash
```

Скрипт автоматически установит зависимости, скачает и запустит клиент.

---

## Структура проекта

```
Truten/
├── server/          # C++ сервер (Crow, libpqxx, jwt-cpp)
│   ├── src/
│   ├── include/
│   └── CMakeLists.txt
├── client/          # Qt6/QML клиент
│   ├── src/
│   ├── qml/
│   └── CMakeLists.txt
├── .env             # конфигурация БД (создать вручную, не коммитить)
└── README.md
```

---

## Возможные проблемы

**CMake не находит Qt6**
```bash
sudo apt install qt6-base-dev qt6-declarative-dev \
    qml6-module-qtquick qml6-module-qtquick-controls \
    qml6-module-qtquick-layouts
```

**Ошибка подключения к БД при запуске сервера**
```bash
sudo systemctl status postgresql
# Проверить данные в .env — DB_USER, DB_PASS, DB_NAME
```

**`libpqxx` не найден при сборке**
```bash
sudo apt install libpqxx-dev
```

**Клиент не отображается (ошибка дисплея на сервере)**
```bash
# Если запускаете на headless-сервере, нужен виртуальный дисплей:
sudo apt install xvfb
Xvfb :99 -screen 0 1280x800x24 &
DISPLAY=:99 ./client/build/appclient
```
