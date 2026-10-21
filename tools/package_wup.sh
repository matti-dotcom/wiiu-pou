#!/bin/bash
# tools/package_wup.sh
# Packaging script for Linux / macOS / MSYS2

set -e

NUSPACKER_JAR="${NUSPACKER_JAR:-NUSPacker.jar}"
INPUT_DIR="${1:-..}"
OUTPUT_DIR="${2:-../Pou_WUP_Output}"
COMMON_KEY="${WIIU_COMMON_KEY}"

if [ -z "$COMMON_KEY" ]; then
    echo "ERROR: WIIU_COMMON_KEY environment variable is not set."
    echo "Usage: WIIU_COMMON_KEY=<32_hex_chars> ./package_wup.sh [input_dir] [output_dir]"
    exit 1
fi

if [ ! -f "$NUSPACKER_JAR" ]; then
    echo "ERROR: NUSPacker.jar not found at $NUSPACKER_JAR"
    exit 1
fi

echo "Packaging Wii U Title with NUSPacker..."
java -jar "$NUSPACKER_JAR" -in "$INPUT_DIR" -out "$OUTPUT_DIR" -key "$COMMON_KEY"
echo "Package completed: $OUTPUT_DIR"
