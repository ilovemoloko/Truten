#!/bin/bash
set -e

cd "$(dirname "$0")"

mkdir -p client/build
cd client/build
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=/usr/lib/x86_64-linux-gnu/cmake/Qt6 > /dev/null
cmake --build . --parallel $(nproc)
cd ../..

./client/build/appclient
