# Kiosk System

<img src="https://img.freepik.com/free-vector/modern-exhibition-icon_1284-66424.jpg?w=1380&t=st=1704563769~exp=1704564369~hmac=68e55b385bb7ee0496a41827caac25d849b75a04ad47425a7e1d158c85fb3415" height="150" width="150">

A modern, extensible kiosk system built with enterprise-grade C++ technologies. The system leverages:

- **Conan** for robust package management
- **gRPC with Protocol Buffers** for efficient client-server communication  
- **PostgreSQL** for reliable data persistence
- **Buf Schema Registry** for proto definition management

## Overview

Originally developed as a Movie Ticketing System, this project has evolved into a flexible, generic-purpose kiosk platform that can be adapted for various use cases. The system demonstrates modern C++ practices and enterprise architecture patterns.

## Prerequisites

- Python 3 with [pipx](https://pipx.pypa.io/) (or pip)
- [Conan 2](https://conan.io/) (`pipx install "conan>=2,<3"`, then `conan profile detect`)
- CMake 3.22+
- A C++17 compiler (GCC 11+ or Clang 14+)
- Docker (optional, for PostgreSQL and the container image)

All C++ dependencies (gRPC, Protobuf, libpqxx/libpq, nlohmann_json, GoogleTest)
come from Conan, so no system packages are needed beyond a compiler and CMake.

## Building from Source

```bash
git clone https://github.com/vishesh-gupta/kiosk-system.git
cd kiosk-system

make build            # conan install + cmake configure + build into ./build
# or, step by step:
conan install . --output-folder=build --build=missing -s build_type=Release
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=build/conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
```

`make BUILD_TYPE=Debug build` builds the project in Debug while keeping the
(prebuilt) dependencies in Release.

The protobuf and gRPC sources are generated from `proto/kiosk/kiosk.proto`
into `build/gen/` at build time.

## Running

Start PostgreSQL (the schema and seed data in `sql/` are loaded on first start):

```bash
make db               # docker compose up postgres
make run              # ./build/kiosk_server, listens on 0.0.0.0:50051
```

Or run the whole stack in containers:

```bash
make docker           # docker compose up --build (postgres + kiosk server)
```

### Terminal UI

`kiosk_tui` is an interactive terminal client for browsing and editing movies
over gRPC:

```bash
make tui                                  # connects to localhost:50051
./build/kiosk_tui --address host:50051    # or set KIOSK_SERVER=host:50051
```

```
╭────────────────────────────────────────────────────────────────────────────────────────╮
│ KIOSK   Movies  localhost:50051                                               ● online │
├────────────────────────────────────────────────────────────────────────────────────────┤
│ Search type to filter by title or description                                 5 movies │
├───────────────────────────────────────────────────┬────────────────────────────────────┤
│ ID    Title                    Year  Len   Rating │                                    │
├───────────────────────────────────────────────────┤ Pulp Fiction                       │
│ 5     Pulp Fiction            1994  154m  R       │ 1994  ·  154 min  ·  Rated R       │
│ 4     The Dark Knight         2008  152m  PG-13   │ ────────────────────────────────── │
│ 3     Interstellar            2014  169m  PG-13   │ The lives of two mob hitmen        │
│ 2     Inception               2010  148m  PG-13   │ intertwine                         │
│ 1     The Matrix              1999  136m  R       │                                    │
├───────────────────────────────────────────────────┴────────────────────────────────────┤
│ ↑↓ select  / search  n new  e edit  d delete  [ ] page  r refresh  q quit              │
╰────────────────────────────────────────────────────────────────────────────────────────╯
```

| Key            | Action                                             |
|----------------|----------------------------------------------------|
| `↑` `↓`        | Move the selection (mouse works too)               |
| `/`            | Live search by title or description; `Esc` returns |
| `n`            | New movie (form: `Tab` between fields, `Enter` saves, `Esc` cancels) |
| `e` / `Enter`  | Edit the selected movie                            |
| `d` / `Delete` | Delete the selected movie (asks for confirmation)  |
| `[` `]`        | Previous / next page (20 movies per page)          |
| `r`            | Reload the list and re-check server health         |
| `q`            | Quit                                               |

RPCs run on a background thread, so the UI stays responsive on a slow or
unreachable server; the header shows server health and is refreshed every five
seconds.

### Configuration

| Setting           | Source                                                       | Default             |
|-------------------|--------------------------------------------------------------|---------------------|
| Database settings | `config/config.json` (path overridable with `KIOSK_CONFIG`)  | see file            |
| DB overrides      | `DB_HOST`, `DB_PORT`, `DB_NAME`, `DB_USER`, `DB_PASSWORD`    | unset               |
| Listen address    | `KIOSK_ADDRESS`                                              | `0.0.0.0:50051`     |

Environment variables take precedence over the config file.

## API

The `kiosk.Kiosk` service (see `proto/kiosk/kiosk.proto`) offers `GetMovie`,
`ListMovies` (paging via `limit`/`offset`, case-insensitive `search_query` on
name and description), `CreateMovie`, `UpdateMovie`, `DeleteMovie` and
`HealthCheck`.

- Successful calls return `OK` with `success = true`.
- Errors use gRPC status codes: `INVALID_ARGUMENT` (bad input), `NOT_FOUND`,
  `UNAVAILABLE` (database unreachable) and `INTERNAL`.
- `UpdateMovie` is a partial update: empty strings and zero numbers leave the
  existing value unchanged.

The server also exposes gRPC reflection and the standard `grpc.health.v1.Health`
service, so tools like [grpcurl](https://github.com/fullstorydev/grpcurl) work
without the proto file:

```bash
grpcurl -plaintext localhost:50051 list
grpcurl -plaintext -d '{"limit": 2}' localhost:50051 kiosk.Kiosk/ListMovies
grpcurl -plaintext -d '{"name": "Alien", "release_year": 1979, "duration": 117}' \
  localhost:50051 kiosk.Kiosk/CreateMovie
```

## Testing

```bash
make db               # tests need a PostgreSQL with sql/schema.sql loaded
make test
```

Database-backed tests are skipped when PostgreSQL is unreachable; set
`KIOSK_REQUIRE_DB=1` (as CI does) to make them fail instead. The service tests
start the gRPC server in-process and call it through a real client stub.
