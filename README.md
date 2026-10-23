# FlymasterDump
Dump all flights from a Flymaster Gps SD Vario into IGC files.

## Dependencies

* **CMake** 3.10+
* **C++14** compiler (Clang / GCC / MSVC)
* **Qt 5** (Core, Widgets, Gui)
* **Boost** (system, date_time, asio)
* **Google Test** (gtest, gtest_main)

## Build Instructions

### macOS (Apple Silicon / Intel)
```bash
# Install dependencies via Homebrew if needed
brew install qt@5 boost googletest cmake

# Configure and build
cmake -B build \
  -DCMAKE_PREFIX_PATH="$(brew --prefix qt@5);$(brew --prefix boost);$(brew --prefix googletest)"
cmake --build build -j

# Run tests
ctest --test-dir build --output-on-failure
```

### Nix (Flakes or nix-shell)
```bash
# Using Nix Flakes
nix build
# or run development environment
nix develop

# Or classic nix-shell
nix-shell
```

### Windows (MSVC)
```cmd
cmake -B build
cmake --build build --config Release
```

## Running

The executable is located at `build/app/variodump`.
On macOS, it auto-detects USB serial devices in `/dev/cu.*` and allows manual entry of any serial device path.
