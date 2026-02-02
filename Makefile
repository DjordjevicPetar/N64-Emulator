# Detect OS
ifeq ($(OS),Windows_NT)
    TARGET := n64.exe
    RM := cmd /C del /Q
    RUN_PREFIX :=
else
    TARGET := n64
    RM := rm -f
    RUN_PREFIX := ./
endif

# Find all .cpp files in src/ and subdirectories
SOURCES := $(shell find src -name '*.cpp')
CXX := g++
CXXFLAGS := -std=c++20 -O3 -Wall -I./src $(shell pkg-config --cflags sdl3)
LDFLAGS := $(shell pkg-config --libs sdl3)

.PHONY: clean build run

clean:
	-$(RM) $(TARGET)

build: clean
	$(CXX) $(CXXFLAGS) $(SOURCES) -o $(TARGET) $(LDFLAGS)

run: build
	$(RUN_PREFIX)$(TARGET) $(ARGS)