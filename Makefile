BUILD_DIR  ?= build
BUILD_TYPE ?= Release
JOBS       ?= $(shell nproc 2>/dev/null || echo 4)

TOOLCHAIN := $(BUILD_DIR)/conan_toolchain.cmake

.PHONY: all deps configure build test run tui db db-down docker clean

all: build

# Install C++ dependencies with Conan (re-runs when conanfile.py changes).
deps: $(TOOLCHAIN)

$(TOOLCHAIN): conanfile.py
	conan install . --output-folder=$(BUILD_DIR) --build=missing \
		-s build_type=Release -s "&:build_type=$(BUILD_TYPE)"

configure: $(TOOLCHAIN)
	cmake -S . -B $(BUILD_DIR) \
		-DCMAKE_TOOLCHAIN_FILE=$(TOOLCHAIN) \
		-DCMAKE_BUILD_TYPE=$(BUILD_TYPE)

build: configure
	cmake --build $(BUILD_DIR) --parallel $(JOBS)

test: build
	ctest --test-dir $(BUILD_DIR) --output-on-failure

run: build
	./$(BUILD_DIR)/kiosk_server

# Terminal UI; point it elsewhere with `make tui ADDRESS=host:port`.
ADDRESS ?= localhost:50051
tui: build
	./$(BUILD_DIR)/kiosk_tui --address $(ADDRESS)

# Start only PostgreSQL (schema and seed data are loaded on first start).
db:
	docker compose -f .docker/docker-compose.yml up -d --wait postgres

db-down:
	docker compose -f .docker/docker-compose.yml down

docker:
	docker compose -f .docker/docker-compose.yml up --build

clean:
	rm -rf $(BUILD_DIR)
