# qr-gen

A lightweight, zero-dependency C++ command-line QR Code generator that rasterizes text payloads into PNG images.

Built on [Project Nayuki's](https://github.com/nayuki) [QR-Code-generator](https://github.com/nayuki/QR-Code-generator) and [tiny-png-output](https://github.com/nayuki/Nayuki-web-published-code/tree/master/tiny-png-output), inspired by [Daniel Hirsch's](https://www.youtube.com/@HirschDaniel) tutorial [Coding a QR Code Generator in C](https://www.youtube.com/watch?v=duchBA8niWg).

---

## Features

- **Zero External Dependencies:** Built with pure C++17 standard library and vendored single-source headers.
- **Optimized Scanline Caching:** Pre-computes RGB scanlines per QR module row, reducing matrix queries by $100\times$ (at scale 10) with minimal memory footprint (~55 KB for Version 40 QR).
- **Decoupled Architecture:** Clean separation between the domain rasterizer (`qrgen::savePng`) and the CLI presentation layer.
- **Reliable POSIX CLI:** Robust argument parser accepting flags in any position, input bounds validation (`1 <= scale <= 100`), and reliable I/O stream error detection (preventing silent write failure on full disks or broken pipes).
- **Comprehensive Verification:** Integrated zero-dependency C++ unit test runner (`test_domain`) and CLI regression test suite verified under AddressSanitizer (ASan) and UndefinedBehaviorSanitizer (UBSan).

---

## Building

### Prerequisites

- C++17 compliant compiler (`g++` 10+ or `clang++` 11+)
- Either `cmake` (3.20+) or GNU `make`

### Building with GNU Make

```bash
# Build the qr-gen binary
make

# Run the full test suite
make test

# Clean artifacts
make clean
```

### Building with CMake

```bash
# Configure and build
cmake -B build -S .
cmake --build build

# Run unit tests and regression suite via CTest
ctest --test-dir build --output-on-failure
```

To build with AddressSanitizer and UndefinedBehaviorSanitizer enabled:

```bash
cmake -B build -S . -DENABLE_SANITIZERS=ON
cmake --build build
ctest --test-dir build --output-on-failure
```

---

## Usage

```bash
qr-gen <text to encode> [OPTIONS]
```

### Options

| Option | Description | Default |
| :--- | :--- | :--- |
| `-o, --output <file>` | Path to destination PNG file | `qrcode.png` |
| `-s, --scale <n>` | Pixel multiplier per QR module (range: `1-100`) | `10` |
| `-h, --help` | Display usage information and exit | — |

### Examples

```bash
# Basic QR code generation:
./qr-gen "https://github.com/tpsapp/qr-gen"

# Custom output destination and high-resolution scale:
./qr-gen "https://example.com" -o myqr.png -s 20

# Options can appear in any order:
./qr-gen -s 15 -o wifi.png "WIFI:S:MyNetwork;T:WPA;P:SecretPassword;;"
```

---

## Project Structure

```
qr-gen/
├── CMakeLists.txt        # Modern CMake build and test configuration
├── Makefile              # Lean Makefile for quick builds and tests
├── main.cpp              # CLI entry point, argument parsing, console diagnostics
├── qr_renderer.hpp       # Decoupled domain PNG renderer interface
├── qr_renderer.cpp       # Scanline caching rasterizer and stream verification
├── qrcodegen.hpp/.cpp    # Project Nayuki QR Code generator library (MIT)
├── tinypngout.hpp/.cpp   # Project Nayuki Tiny PNG Output library (LGPLv3)
├── tests/
│   ├── TestHarness.hpp   # Header-only zero-dependency C++ unit test framework
│   ├── test_domain.cpp   # Domain boundary tests and I/O fault injection
│   └── test_regression.sh# Black-box CLI parameter and argument ordering tests
├── CONTRIBUTING.md       # Contribution guidelines
├── SECURITY.md           # Security policy and vulnerability disclosure
└── LICENSE               # GNU General Public License v3
```

---

## License

This project is licensed under the [GNU General Public License v3 (GPLv3)](LICENSE).

Third-party components:
- `qrcodegen` &copy; Project Nayuki (MIT License)
- `tinypngout` &copy; Project Nayuki (GNU Lesser General Public License v3)
