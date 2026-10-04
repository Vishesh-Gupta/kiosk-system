# syntax=docker/dockerfile:1

# ---- Build stage -------------------------------------------------------------
FROM ubuntu:24.04 AS build

RUN apt-get update && \
    apt-get install -y --no-install-recommends \
        build-essential cmake git pipx python3 ca-certificates && \
    rm -rf /var/lib/apt/lists/*

ENV PATH="/root/.local/bin:${PATH}"
RUN pipx install "conan>=2,<3" && conan profile detect --force

WORKDIR /kiosk

# Resolve dependencies first so this layer is cached until conanfile.py changes.
COPY conanfile.py .
RUN --mount=type=cache,target=/root/.conan2/p \
    conan install . --output-folder=build --build=missing -s build_type=Release

COPY CMakeLists.txt .
COPY proto/ ./proto/
COPY src/ ./src/
COPY tests/ ./tests/

RUN --mount=type=cache,target=/root/.conan2/p \
    cmake -S . -B build \
        -DCMAKE_TOOLCHAIN_FILE=build/conan_toolchain.cmake \
        -DCMAKE_BUILD_TYPE=Release \
        -DKIOSK_BUILD_TESTS=OFF && \
    cmake --build build --parallel && \
    cmake --install build --prefix /opt/kiosk

# ---- Runtime stage -----------------------------------------------------------
# Conan links gRPC, protobuf, libpq and OpenSSL statically, so the runtime image
# only needs the C/C++ runtime that ubuntu ships.
FROM ubuntu:24.04

RUN groupadd --system kiosk && useradd --system --gid kiosk kiosk

COPY --from=build /opt/kiosk/bin/kiosk_server /usr/local/bin/kiosk_server
COPY config/config.json /etc/kiosk/config.json

ENV KIOSK_CONFIG=/etc/kiosk/config.json \
    KIOSK_ADDRESS=0.0.0.0:50051

USER kiosk
EXPOSE 50051

ENTRYPOINT ["/usr/local/bin/kiosk_server"]
