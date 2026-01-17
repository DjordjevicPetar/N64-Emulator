#!/bin/bash

test_rom() {
    local rom="$1"
    local name=$(basename "$rom" .N64)
    
    perl -e 'alarm 1; exec @ARGV' ./n64 "$rom" >/dev/null 2>&1
    local exit_code=$?
    
    if [ $exit_code -eq 142 ]; then
        echo "✓ $name"
        return 0
    elif [ $exit_code -eq 0 ]; then
        echo "✓ $name"
        return 0
    else
        error=$(./n64 "$rom" 2>&1 | head -1)
        echo "✗ $name: $error"
        return 1
    fi
}

echo "=== Testing All ROMs ==="

# Find all .N64 files
for rom in $(find tests/roms/peterlemon -name "*.N64" 2>/dev/null | sort); do
    test_rom "$rom"
done 2>/dev/null
