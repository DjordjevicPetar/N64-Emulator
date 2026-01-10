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

SOURCES := $(wildcard *.cpp)
CXX := g++
CXXFLAGS := -std=c++20 -O3 -Wall -I./inc

.PHONY: clean build run

clean:
	-$(RM) $(TARGET)

build: clean
	$(CXX) $(CXXFLAGS) $(SOURCES) -o $(TARGET)

run: build
	$(RUN_PREFIX)$(TARGET) $(ARGS) -l