#!/bin/bash
set -e

echo "==> Установка зависимостей..."
sudo apt update -qq
sudo apt install -y \
    git cmake g++ \
    qt6-base-dev qt6-declarative-dev \
    qml6-module-qtquick qml6-module-qtquick-controls \
    qml6-module-qtquick-layouts

echo "==> Скачивание клиента..."
git clone --depth=1 --branch clientStuff https://github.com/ilovemoloko/Truten.git ~/Truten 2>/dev/null \
    || git -C ~/Truten pull

echo "==> Сборка..."
mkdir -p ~/Truten/client/build
cmake -S ~/Truten/client -B ~/Truten/client/build -DCMAKE_BUILD_TYPE=Release > /dev/null
cmake --build ~/Truten/client/build --parallel $(nproc)

echo "==> Запуск..."
~/Truten/client/build/appclient
