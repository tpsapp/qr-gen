#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" &>/dev/null && pwd -P)"
PROJECT_ROOT="$(cd -- "${SCRIPT_DIR}/.." &>/dev/null && pwd -P)"
BIN="${PROJECT_ROOT}/qr-gen"

TMP_DIR="$(mktemp -d /tmp/qr-gen-test.XXXXXX)"
cleanup() {
    rm -rf "${TMP_DIR}"
}
trap cleanup EXIT

echo "Running regression tests against ${BIN}..."

if [[ ! -x "${BIN}" ]]; then
    echo "Error: Binary ${BIN} not found or not executable. Build it first." >&2
    exit 1
fi

passed=0
failed=0

assert_success() {
    local desc="$1"
    shift
    if "$@" >/dev/null 2>&1; then
        echo "  [PASS] ${desc}"
        passed=$((passed + 1))
    else
        echo "  [FAIL] ${desc} (command failed unexpectedly)"
        failed=$((failed + 1))
    fi
}

assert_failure() {
    local desc="$1"
    shift
    if "$@" >/dev/null 2>&1; then
        echo "  [FAIL] ${desc} (expected failure, but succeeded)"
        failed=$((failed + 1))
    else
        echo "  [PASS] ${desc}"
        passed=$((passed + 1))
    fi
}

# --- FIX-03: CLI Argument Order & Permutations ---
echo "Testing CLI Argument Parsing & Ordering (FIX-03)..."
assert_success "Help flag (--help) exits 0" "${BIN}" --help
assert_success "Help flag (-h) exits 0" "${BIN}" -h
assert_failure "No arguments exits non-zero" "${BIN}"

assert_success "Standard order: text -o file" "${BIN}" "Test 1" -o "${TMP_DIR}/out1.png"
assert_success "Option before text: -o file text" "${BIN}" -o "${TMP_DIR}/out2.png" "Test 2"
assert_success "Interleaved options: -s 5 text -o file" "${BIN}" -s 5 "Test 3" -o "${TMP_DIR}/out3.png"
assert_success "Trailing options: text -s 3 -o file" "${BIN}" "Test 4" -s 3 -o "${TMP_DIR}/out4.png"

assert_failure "Missing argument for -o exits non-zero" "${BIN}" "Test" -o
assert_failure "Missing argument for -s exits non-zero" "${BIN}" "Test" -s
assert_failure "Unrecognized option exits non-zero" "${BIN}" "Test" -z
assert_failure "Multiple text arguments exit non-zero" "${BIN}" "Test" "Extra"

# --- FIX-02: Integer Overflow & Scale Validation ---
echo "Testing Scale Validation & Overflow Prevention (FIX-02)..."
assert_failure "Scale 0 rejected" "${BIN}" "Test" -s 0 -o "${TMP_DIR}/scale0.png"
assert_failure "Negative scale rejected" "${BIN}" "Test" -s -5 -o "${TMP_DIR}/scaleneg.png"
assert_failure "Non-numeric scale rejected" "${BIN}" "Test" -s abc -o "${TMP_DIR}/scaleabc.png"
assert_failure "Trailing garbage in scale rejected" "${BIN}" "Test" -s 10x -o "${TMP_DIR}/scalegarbage.png"
assert_failure "Scale > 100 rejected" "${BIN}" "Test" -s 101 -o "${TMP_DIR}/scale101.png"
assert_failure "Extreme scale (100000000) rejected without UBSan crash" "${BIN}" "Test" -s 100000000 -o "${TMP_DIR}/scalehuge.png"
assert_success "Valid boundary scale 1 accepted" "${BIN}" "Test" -s 1 -o "${TMP_DIR}/scale1.png"
assert_success "Valid boundary scale 100 accepted" "${BIN}" "Test" -s 100 -o "${TMP_DIR}/scale100.png"

# --- FIX-01: Silent Write Failure on Disk Full ---
echo "Testing I/O Error Detection & Full Disk Handling (FIX-01)..."
if [[ -e /dev/full ]]; then
    assert_failure "Writing to /dev/full reports failure and exits non-zero" "${BIN}" "Test" -o /dev/full
fi

# --- Functional PNG Verification ---
echo "Testing PNG File Integrity..."
"${BIN}" "Nayuki QR Code" -s 4 -o "${TMP_DIR}/verify.png"
header=$(head -c 8 "${TMP_DIR}/verify.png")
expected=$(printf '\x89PNG\r\n\x1a\n')
if [[ "${header}" == "${expected}" ]]; then
    echo "  [PASS] Output PNG has valid 8-byte PNG magic signature"
    passed=$((passed + 1))
else
    echo "  [FAIL] Output PNG magic signature mismatch"
    failed=$((failed + 1))
fi

echo "=================================="
echo "Tests Passed: ${passed}, Failed: ${failed}"
if [[ ${failed} -ne 0 ]]; then
    exit 1
fi
