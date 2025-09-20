#!/bin/bash

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
OUTPUT_DIR="$ROOT_DIR/apps/TakeADump"

if [ ! -e "$ROOT_DIR/TakeADump.dol" ]; then
    echo "Please run the Makefile in the root directory."
    exit 1
fi

echo "Using output $OUTPUT_DIR"...
if [ -d "$OUTPUT_DIR" ]; then rm -rf "$OUTPUT_DIR"; fi
mkdir -p "$OUTPUT_DIR"
echo "Copying files..."

if [ -e "$ROOT_DIR/release/icon.png" ]; then cp "$ROOT_DIR/release/icon.png" "$OUTPUT_DIR/icon.png"; fi
cp "$ROOT_DIR/release/meta.xml" "$OUTPUT_DIR/meta.xml"
cp "$ROOT_DIR/TakeADump.dol" "$OUTPUT_DIR/boot.dol"

echo "Process complete!"
exit 0