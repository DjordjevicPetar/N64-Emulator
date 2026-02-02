#!/bin/bash

# N64 Emulator Test Runner
# Runs each test ROM for 5 seconds

EMULATOR="./n64"
TEST_DIR="tests/roms/peterlemon/CPUTest/CPU"  # Only CPU tests
TIMEOUT=5  # seconds per test

# Build first
echo "Building emulator..."
make build || exit 1

# Find all test ROMs
ROMS=$(find "$TEST_DIR" -name "*.N64" | sort)
TOTAL=$(echo "$ROMS" | wc -l | tr -d ' ')

echo "Found $TOTAL CPU test ROMs"
echo ""

COUNT=0
for ROM in $ROMS; do
    COUNT=$((COUNT + 1))
    TEST_NAME=$(basename "$ROM" .N64)
    
    printf "[%3d/%3d] %-40s " "$COUNT" "$TOTAL" "$TEST_NAME"
    
    # Run in background, kill after timeout
    "$EMULATOR" "$ROM" 2>/dev/null &
    PID=$!
    
    sleep $TIMEOUT
    
    # Check if still running
    if kill -0 $PID 2>/dev/null; then
        kill $PID 2>/dev/null
        wait $PID 2>/dev/null
        echo "OK"
    else
        wait $PID
        EXIT_CODE=$?
        echo "CRASH (exit $EXIT_CODE)"
    fi
done

echo ""
echo "Done!"
