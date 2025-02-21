CC = g++
CFLAGS = -std=c++17 -Wall -Wextra -O2
LDFLAGS = -lboost_system -lboost_filesystem -lprotobuf -lgrpc -lpq

SRC = $(wildcard src/*.cc)
OBJ = $(SRC:.cc=.o)
TARGET = kiosk

# Conan configuration
CONAN = conan
CONAN_OPTIONS = --build=missing

# Targets
all: install $(TARGET)

install:
	$(CONAN) install . $(CONAN_OPTIONS) --output-folder=build

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $@ $(LDFLAGS)

%.o: %.cc
	$(CC) $(CFLAGS) -c $< -o $@

# Testing
test: $(TARGET)
	./tests/run_tests

# Linting
lint:
	clang-tidy --exclude-dir=/usr/local $(SRC)

# Formatting
format:
	clang-format -i $(SRC)

# Clean up
clean:
	rm -f $(OBJ) $(TARGET)

.PHONY: all clean test lint format conan_install
