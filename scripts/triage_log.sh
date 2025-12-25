#!/usr/bin/env bash
set -euo pipefail

LOG="build/qemu.log"

if [[ ! -f "$LOG" ]]; then
  echo "[triage] missing $LOG" >&2
  exit 1
fi

echo "[triage] tail -n 200 $LOG"
tail -n 200 "$LOG" || true

echo

echo "[triage] interesting patterns"
# Best-effort: qemu log format varies by version/flags
grep -nE "check_exception|Servicing hardware INT|#UD|#GP|#DF|triple fault|Triple fault|double fault" "$LOG" || true
