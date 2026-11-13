# Contributing to qr-gen

Thank you for your interest in contributing to `qr-gen`!

## Code of Conduct

Please be respectful, constructive, and collaborative in all project interactions.

## How to Contribute

1. **Reporting Issues:**
   - Check existing issues to see if your bug or feature request has already been logged.
   - Provide minimal reproduction steps, expected behavior, and actual behavior.
   - For security vulnerabilities, refer to [SECURITY.md](SECURITY.md).

2. **Developing Changes:**
   - Clone the repository and ensure you have a C++17 compliant compiler (`g++` or `clang++`) and `cmake` (3.20+) or `make`.
   - Adhere to the core engineering philosophy:
     - **Standard Library Primacy:** Zero external dependencies.
     - **Explicit Control Flow:** Guard clauses and early returns.
     - **Domain Decoupling:** Keep core QR rasterization decoupled from presentation/CLI handlers.

3. **Building & Running Tests:**
   ```bash
   # Build and run the test suites using Makefile:
   make test

   # Or build with CMake and memory sanitizers enabled:
   cmake -B build -S . -DENABLE_SANITIZERS=ON
   cmake --build build
   ctest --test-dir build --output-on-failure
   ```
   All domain unit tests and regression assertions must pass with zero sanitizer warnings before submitting.

4. **Pull Requests:**
   - Submit PRs targeting the `main` branch.
   - Use clear commit messages adhering to Conventional Commits (e.g., `feat:`, `fix:`, `docs:`, `test:`).
