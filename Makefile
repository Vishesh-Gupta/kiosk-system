CC = g++
CFLAGS = -std=c++17 -Wall -Wextra -O2
LDFLAGS = -lboost_system -lboost_filesystem -lprotobuf -lgrpc -lpq

SRC = $(wildcard src/*.cc)
OBJ = $(SRC:.cc=.o)
TARGET = kiosk

# Conan configuration
CONAN = conan
CONAN_OPTIONS = --build=missing

CONAN_TOOLCHAIN_FILE = $(pwd)/build/conan_toolchain.cmake
CONAN_PREFIX_PATH = build

# Targets
all: install $(TARGET)

install:
	@$(CONAN) install . $(CONAN_OPTIONS) --output-folder=$(CONAN_PREFIX_PATH)

build: $(CONAN_TOOLCHAIN)
	@cmake -B build \
	 -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
	 -DCMAKE_TOOLCHAIN_FILE=$(CONAN_TOOLCHAIN_FILE) \
	 -DCMAKE_PREFIX_PATH=$(CONAN_PREFIX_PATH) \
	 -DCMAKE_BUILD_TYPE=Release
	@cmake --build build --config Release

$(CONAN_TOOLCHAIN): conanfile.py
	@echo "Conan toolchain not found or outdated. Installing dependencies..."
	$(CONAN) install . $(CONAN_OPTIONS) --output-folder=build

# Clean up
clean:
	rm -f $(OBJ) $(TARGET)

.PHONY: all clean build install
