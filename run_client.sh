#!/bin/bash
set -e

echo "<=> Установка зависимостей..."
sudo apt update -qq
sudo apt install -y \
    git cmake g++ \
    qt6-base-dev qt6-declarative-dev \
    qml6-module-qtquick qml6-module-qtquick-controls \
    qml6-module-qtquick-layouts qml6-module-qtquick-window \
    qml6-module-qtqml-workerscript qml6-module-qtquick-templates \
    qml6-module-qtquick-nativestyle qml6-module-qtquick-shapes

echo "<=> Сборка клиента..."
mkdir -p client/build
cmake -S client -B client/build -DCMAKE_BUILD_TYPE=Release
cmake --build client/build --parallel $(nproc)

echo "<=> Запуск..."
./client/build/appclient
