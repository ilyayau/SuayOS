#!/usr/bin/env bash
set -euo pipefail

root_dir=$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)
elf="${root_dir}/build/kernel.elf"

if [[ $# -ne 1 ]]; then
  echo "usage: $0 <rip>" >&2
  echo "example: $0 0x100ABC" >&2
  exit 2
fi

addr_raw="$1"
addr_hex="${addr_raw#0x}"

if [[ ! -f "$elf" ]]; then
  echo "error: missing $elf (build it with 'make kernel' or 'make iso')" >&2
  exit 1
fi

addr_dec=$((16#${addr_hex}))

addr2line_tool=""
if command -v llvm-addr2line >/dev/null 2>&1; then
  addr2line_tool="llvm-addr2line"
elif command -v addr2line >/dev/null 2>&1; then
  addr2line_tool="addr2line"
fi

if [[ -n "$addr2line_tool" ]]; then
  if [[ "$addr2line_tool" == "llvm-addr2line" ]]; then
    "$addr2line_tool" -e "$elf" -fip "0x${addr_hex}" || true
  else
    "$addr2line_tool" -e "$elf" -f -C -i -p "0x${addr_hex}" || true
  fi
else
  echo "warning: neither llvm-addr2line nor addr2line found" >&2
fi

nm_tool=""
if command -v llvm-nm >/dev/null 2>&1; then
  nm_tool="llvm-nm"
elif command -v nm >/dev/null 2>&1; then
  nm_tool="nm"
fi

if [[ -n "$nm_tool" ]]; then
  echo
  echo "Nearest symbols:" 
  "$nm_tool" -n "$elf" | awk -v target="${addr_dec}" '
    function h2d(h,   i,c,v){
      v=0
      for(i=1;i<=length(h);i++){
        c=substr(h,i,1)
        if(c>="0"&&c<="9") v=v*16+(c-"0")
        else if(c>="a"&&c<="f") v=v*16+(10+index("abcdef",c)-1)
        else if(c>="A"&&c<="F") v=v*16+(10+index("ABCDEF",c)-1)
        else return -1
      }
      return v
    }
    {
      addr=h2d($1)
      if(addr>=0 && addr<=target){ last=$0 }
    }
    END{ if(last!="") print last; else print "(no symbol <= address)" }
  '
fi
