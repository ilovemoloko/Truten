#!/bin/bash
set -e

echo "==> Подготовка серверной части (Docker)..."
if [ ! -f .env ]; then
    echo "==> Создание .env для сервера..."
    cat > .env <<EOF
DB_HOST=db
DB_PORT=5432
DB_NAME=truten
DB_USER=truten_user
DB_PASS=yourpassword
JWT_SECRET=$(openssl rand -hex 32)
SMTP_HOST=smtp.gmail.com
SMTP_PORT=587
SMTP_USER=your_email@gmail.com
SMTP_PASS=your_app_password
SMTP_FROM=Truten <some@gmail.com>
EOF
    echo "!!! Файл .env создан. Отредактируйте его, чтобы настроить почту  !!!"
fi

echo "<=> Сборка Docker образов..."
docker compose build server

echo "Теперь вы можете запустить сервер командой ./run_server.sh"
