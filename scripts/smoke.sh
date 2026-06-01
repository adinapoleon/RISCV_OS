#!/usr/bin/env sh
set -eu

KERNEL="${1:-kernel.elf}"
TIMEOUT_SECONDS="${SMOKE_TIMEOUT_SECONDS:-5}"
OUTPUT_FILE="${SMOKE_OUTPUT_FILE:-$(mktemp)}"

if ! command -v qemu-system-riscv32 >/dev/null 2>&1; then
    echo "qemu-system-riscv32 not found" >&2
    exit 1
fi

if [ ! -f "$KERNEL" ]; then
    echo "kernel image not found: $KERNEL" >&2
    exit 1
fi

set +e
timeout "${TIMEOUT_SECONDS}s" qemu-system-riscv32 \
    -machine virt \
    -nographic \
    -bios none \
    -kernel "$KERNEL" >"$OUTPUT_FILE" 2>&1
status=$?
set -e

cat "$OUTPUT_FILE"

if [ "$status" -ne 0 ] && [ "$status" -ne 124 ]; then
    echo "QEMU exited unexpectedly with status $status" >&2
    exit "$status"
fi

if grep -q "\[FAIL\]" "$OUTPUT_FILE"; then
    echo "Kernel smoke checks reported a failure" >&2
    exit 1
fi

grep -q "PMM smoke test passed." "$OUTPUT_FILE"
grep -q "VMM smoke test passed." "$OUTPUT_FILE"
grep -q "Sv32 paging enabled" "$OUTPUT_FILE"
grep -q "S-mode privilege verified." "$OUTPUT_FILE"
grep -q "Timer tick" "$OUTPUT_FILE"
grep -q "Timer smoke test passed." "$OUTPUT_FILE"
grep -q "Syscall smoke test passed." "$OUTPUT_FILE"
grep -q "Memory bring-up complete." "$OUTPUT_FILE"
grep -q "Kernel idle." "$OUTPUT_FILE"

echo "Smoke test passed."
