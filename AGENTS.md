# AGENTS.md — Instructions and Context for AI Agents in FlymasterDump

## 1. Project Overview

**FlymasterDump** is a cross-platform desktop C++ application with a graphical user interface based on Qt 5.
**Purpose:** Read flight logs from a Flymaster variometer (GPS SD Vario) via serial communication (Serial / USB-UART) and export tracks into IGC (`.igc`) file format.

### Tech Stack:
- **Language:** C++14
- **GUI & Core:** Qt 5 (Widgets, Core, Gui)
- **Networking & Async I/O:** Boost.Asio (including `boost_system`, `boost_date_time`)
- **Testing:** Google Test (GTest)
- **Build System:** CMake (3.10+)
- **Environments & Packaging:** Homebrew (macOS) and Nix (Flakes, `nix-shell`) support

---

## 2. Architecture and Codebase Structure

```
FlymasterDump/
├── CMakeLists.txt              # Main CMake configuration file
├── flake.nix / shell.nix       # Nix dependencies and environment definitions
├── scripts/
│   └── thirdparty.cmake        # Third-party library configuration (Qt5, Boost, GTest)
├── progress/                   # Progress reporting library (progressLib)
│   ├── Callback.h / .cpp       # Base callback interface and Severity levels
│   └── Callback.test.cpp       # Unit tests for progressLib
├── serialPortCom/              # Flymaster protocol & serial communication library (serialPortComLib)
│   ├── SerialPort.h / .cpp     # COM / serial port operations using Boost.Asio
│   ├── Request.h / .cpp        # High-level requests (serial number, flight list, download)
│   ├── IgcFile.h / .cpp        # IGC file data structure
│   ├── IgcFactory.h / .cpp     # Converts Flymaster binary packets into IGC format
│   ├── ascii/                  # Flymaster ASCII/NMEA protocol ($PFMSNP, $PFMDNL, etc.)
│   │   ├── FlightMetaInfo.*    # Metadata of an individual flight
│   │   ├── NmeaSentence.*      # NMEA parsing and checksum computation
│   │   └── StringUtils.*       # String splitting and joining utilities
│   └── binary/                 # Flymaster binary transmission protocol
│       ├── BinaryBlock.*       # Base binary block with CRC
│       ├── Convert.*           # Little-endian number conversions
│       ├── FlightInformationRecord.* # Device information record
│       ├── KeyTrackPositionRecord.*  # Base GPS track point record
│       └── TrackPositionRecordDeltas.*# Delta-compressed track points
└── app/                        # Main application executable (variodump) and UI (varioDumpUiLib)
    ├── main.cpp                # QApplication entry point
    ├── MainWindow.h / .cpp / .ui # Main window and UI controls
    ├── Model.h / .cpp          # Configuration model (port, baud rate, pilot metadata)
    ├── Property.h              # Reactive property wrapper with Qt signals
    ├── QtCallback.h / .cpp     # Bridge between Task callback and Qt events/progress dialog
    ├── TaskProcessor.h / .cpp  # Async task dispatcher running on a background thread
    ├── TaskProgressDialog.*    # Modal dialog displaying task progress
    ├── CheckDeviceTask.*       # Task to verify connection and fetch device information
    ├── FlightListTask.*        # Task to fetch the flight list
    ├── DumpTask.*              # Task to download track data and save to .igc
    └── TaskProcessor.test.cpp  # Unit tests for the async task processor
```

---

## 3. Build, Run, and Test

### 3.1. Build with CMake (macOS / Linux)
With dependencies installed (e.g., via Homebrew):
```bash
# Configure with Apple Clang or system compiler
cmake -B build \
  -DCMAKE_C_COMPILER=/usr/bin/clang \
  -DCMAKE_CXX_COMPILER=/usr/bin/clang++ \
  -DCMAKE_PREFIX_PATH="/opt/homebrew/opt/qt@5;/opt/homebrew/opt/boost;/opt/homebrew/opt/googletest"

# Compile
cmake --build build -j

# Run all tests
ctest --test-dir build --output-on-failure
```

### 3.2. Build & Develop with Nix
```bash
# Build package using Nix Flakes
nix build

# Or launch development environment with all dependencies
nix develop
# or
nix-shell
```

### 3.3. Running the Application
The executable binary is generated at:
```bash
./build/app/variodump
```
To verify startup without opening a GUI window (headless):
```bash
./build/app/variodump -platform offscreen
```

---

## 4. Development Guidelines & Important Gotchas

1. **Cross-Platform Compatibility (Avoid Windows/MSVC-specific code):**
   - Do not use MSVC-only functions such as `_itoa_s`, `_putenv`, or `sprintf_s`. Use standardized C++ alternatives (`snprintf`, `setenv` with `tzset()` on POSIX).
   - MSVC-specific linker flags (such as `/ENTRY:mainCRTStartup` or definitions like `_WIN32_WINNT`) must be guarded with `if(MSVC)` or `if(WIN32)`.
   - File name case sensitivity: Header inclusions must strictly match the file name on disk (e.g. `#include "TaskProcessor.h"`, not `"taskProcessor.h"`).

2. **Modern Boost.Asio:**
   - Use `boost::asio::io_context` instead of deprecated `io_service`.
   - Use `boost::asio::steady_timer` and `std::chrono::milliseconds` instead of deprecated `deadline_timer` and `posix_time`.
   - Prefer modern C++11/14 lambdas over `boost::bind` to avoid conflicts with `_1` placeholders and legacy macros.

3. **Serial Port Specifics:**
   - On macOS, serial ports typically follow patterns like `/dev/cu.usbserial*` or `/dev/cu.SLAB_USBtoUART`.
   - The port selection dropdown in the UI must remain editable (`setEditable(true)`) to allow connections to PTY/virtual ports or devices connected after app launch.

4. **GoogleTest:**
   - Tests using `QObject` classes or registering types with Qt's meta-system (like `variodumpTest`) must initialize `QCoreApplication app(argc, argv);` inside `main()`.
   - In `CMakeLists.txt`, always pass the CMake target name to `add_test(NAME ... COMMAND ...)` instead of hardcoded relative paths.

5. **Code Formatting:**
   - Adhere to the code formatting rules defined in the root [`.clang-format`](.clang-format) file.
