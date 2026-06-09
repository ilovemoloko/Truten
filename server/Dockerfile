FROM ubuntu:22.04 AS builder

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    pkg-config \
    libpqxx-dev \
    libssl-dev \
    libasio-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY . .

RUN mkdir -p server/build && cd server/build && \
    cmake .. -DCMAKE_BUILD_TYPE=Release && \
    cmake --build . --parallel $(nproc)

FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    libpqxx-6.4 \
    libssl3 \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app/bin
COPY --from=builder /app/server/build/Server /app/bin/Server

EXPOSE 8080
CMD ["./Server"]
