FROM alpine:3.17.0 AS build

RUN apk update && \
    apk add --no-cache \
        build-base=0.5-r3 \
        cmake=3.24.3-r0 \
        boost1.80-dev=1.80.0-r3 \
        postgresql-dev=15.1-r0 \
        protobuf-dev=3.21.9-r0 \
        grpc-dev=1.50.1-r0

WORKDIR /kiosk

COPY proto/ ./proto/
COPY src/ ./src/
COPY CMakeLists.txt .
COPY conanfile.py .

RUN conan install . --output-folder=build --build=missing

WORKDIR /kiosk/build

RUN cmake -DCMAKE_BUILD_TYPE=Release .. && \
    cmake --build . --parallel 8

FROM alpine:3.17.0

RUN apk update && \
    apk add --no-cache \
    libstdc++=12.2.1_git20220924-r4 \
    boost1.80-program_options=1.80.0-r3 \
    postgresql-libs=15.1-r0 \
    protobuf=3.21.9-r0 \
    grpc=1.50.1-r0

RUN addgroup -S kiosk && adduser -S kiosk -G kiosk
USER kiosk

COPY --chown=kiosk:kiosk --from=build \
    /kiosk/build/kiosk \
    /usr/local/bin/

EXPOSE 50051

ENTRYPOINT [ "/usr/local/bin/kiosk" ]