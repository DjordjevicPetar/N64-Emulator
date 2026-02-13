#!/bin/bash
# Run all RSP tests from Peter Lemon test suite
# Tests RSP CPU instructions, CP2 (Vector Unit), DMA, IMEM, and XBUS

EMULATOR="./n64"
TEST_DIR="tests/roms/peterlemon-all/RSPTest"
TIMEOUT=2

# Build first
echo "Building emulator..."
make build || exit 1

# Find all test ROMs
ROMS=$(find "$TEST_DIR" -name "*.N64" -o -name "*.n64" | sort)
TOTAL=$(echo "$ROMS" | wc -l | tr -d ' ')

echo ""
echo "=== Running $TOTAL RSP Tests ==="
echo ""

COUNT=0
CRASHED=0
for ROM in $ROMS; do
    COUNT=$((COUNT + 1))
    TEST_NAME=$(basename "$ROM" .N64)
    TEST_NAME=$(basename "$TEST_NAME" .n64)
    
    printf "[%3d/%3d] %-45s " "$COUNT" "$TOTAL" "$TEST_NAME"
    
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
        CRASHED=$((CRASHED + 1))
    fi
    
    sleep 0.5
done

echo ""
echo "=== RSP Tests Complete: $((COUNT - CRASHED))/$COUNT OK, $CRASHED crashed ==="
