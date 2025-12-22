#!/bin/sh
# Pack all files in initrd/ into build/initrd.img (flat format: [name:56][size:4][data])
set -e
OUT=build/initrd.img
: > "$OUT"
cd initrd
for f in *; do
  [ -f "$f" ] || continue
  name=$(printf "%-56s" "$f")
  size=$(stat -c %s "$f")
  printf "%s" "$name" >> ../$OUT
  printf "%08x" "$size" | xxd -r -p >> ../$OUT
  cat "$f" >> ../$OUT
  echo "Packed $f ($size bytes)"
done
cd ..
