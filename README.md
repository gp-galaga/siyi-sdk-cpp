# SIYI SDK C++

## Overview

This repository implements SIYI packet encoding/decoding and a ZR30 camera API.

Highlights:
- Packet encode/decode with CRC16-CCITT
- Telecommand helpers for common SIYI command IDs
- Typed telemetry decode for gimbal attitude/configuration
- CLI demo app for real camera testing over UDP
- Unit tests with doctest


## Build with VS Code CMake Tools

This repository now includes `CMakePresets.json`, so CMake Tools can configure/build directly.

1. Open the folder in VS Code.
2. Run `CMake: Select Configure Preset` and choose `debug` or `release`.
3. Run `CMake: Configure`.
4. Run `CMake: Build`.
5. (Optional) Run `CMake: Run Tests`.

Equivalent terminal commands:

```bash
cmake --preset debug
cmake --build --preset debug
ctest --preset debug
```

---

## Using the SDK as a Library in Another Project

You can integrate this SDK in two ways, depending on your workflow.

### Option A: Add as a Submodule + `add_subdirectory` (Best for active co-development)

#### 1. Add as a submodule

In your project root:

```bash
git submodule add <this-repo-url> external/siyi-sdk-cpp
git submodule update --init --recursive
```

#### 2. Add as a subdirectory in CMake

In your project's `CMakeLists.txt`:

```cmake
add_subdirectory(external/siyi-sdk-cpp)

# Example: link your target to the SDK static library
target_link_libraries(your_target PRIVATE siyi_sdk)
```

`siyi_sdk` publishes its include directory as `PUBLIC`, so extra include paths are usually not required.

### Option B: Install + `find_package` (Best for clean external consumption)

#### 1. Build and install this SDK

From the SDK repository root:

```bash
cmake --preset release
cmake --build --preset release
cmake --install build/release --prefix build/release
```

#### 2. Consume it from another project

In your consumer `CMakeLists.txt`:

```cmake
list(APPEND CMAKE_PREFIX_PATH "/absolute/path/to/siyi-sdk-install")

find_package(siyi_sdk CONFIG REQUIRED)
target_link_libraries(your_target PRIVATE siyi_sdk::siyi_sdk)
```

You can also pass the prefix at configure time instead of editing `CMAKE_PREFIX_PATH`:

```bash
cmake -S . -B build -DCMAKE_PREFIX_PATH=/absolute/path/to/siyi-sdk-install
```

### Include Headers in Your Code

Include the relevant headers from `include/camera/core/`, `include/camera/models/`, etc. For example:

```cpp
#include <camera/models/zr30_camera_model.hpp>
#include <camera/core/camera_command_interface.hpp>
// ...
```

Build your project normally after linking to either `siyi_sdk` (subdirectory mode) or `siyi_sdk::siyi_sdk` (package mode).

---


## Project Tree

```
├── CMakeLists.txt
├── CMakePresets.json
├── README.md
├── apps/
│   └── main.cpp                                    # CLI demo app
├── include/
│   ├── camera/
│   |    ├── core/                                  # Core interfaces and infrastructure
│   |    │   ├── ack_policy.hpp
│   |    │   ├── camera_command_interface.hpp
│   |    │   ├── telecommand_session.hpp
│   |    │   └── telemetry_decoder_interface.hpp
│   |    ├── models/                                # Camera model implementations
│   |    │   ├── shared_camera_model.hpp
│   |    │   ├── optical_zoom_camera_model.hpp
│   |    │   └── zr30_camera_model.hpp
│   |    └── protocol/                              # Generated protocol headers from python script `generate_protocol_headers.py` and protocol definition
│   |        ├── tc_parameter.hpp
│   |        └── tm_parameters.hpp
│   └── helper/
│       └── ilog_manager.hpp
│   └── transport/
│       ├── itransport.hpp
│       ├── tcp_transport.hpp
│       └── udp_transport.hpp
├── protocol/
│   ├── tc/definitions.yaml                         # Telecommand definitions
│   └── tm/definitions.yaml                         # Telemetry definitions
├── scripts/
│   ├── export_tc_tm.sh
│   ├── generate_protocol_headers.py
│   └── run_tests.sh
├── src/
│   ├── camera/
│   │   ├── shared_camera_model.cpp
│   │   ├── optical_zoom_camera_model.cpp
│   │   └── zr30_camera_model.cpp
│   ├── helper/
│   │   └── log_manager.cpp
│   └── transport/
│       ├── tcp_transport.cpp
│       └── udp_transport.cpp
├── tests/
│   ├── doctest.h
│   ├── test_shared_camera_model.cpp
│   ├── test_optical_zoom_camera_model.cpp
│   └── test_zr30_camera_model.cpp
```

---

## Running Unit Tests (Manual)

You can run all unit tests manually using CMake presets or the provided script. The tests use [doctest](https://github.com/doctest/doctest).

**Recommended (from repo root):**

```bash
python3 scripts/generate_protocol_headers.py 
cmake --preset debug
cmake --build --preset debug
ctest --preset debug
```

Or run the test binary directly:

```bash
./build/debug/test_siyi_commands
```

Or use the helper script:

```bash
bash scripts/run_tests.sh
```

All tests should pass. If you add or modify tests, rebuild and rerun as above.

---

## Protocol Definitions as YAML

Command and telemetry definitions are maintained in:

- `protocol/tc/definitions.yaml`
- `protocol/tm/definitions.yaml`

`protocol/tm/definitions.yaml` defines each TM packet using:

- `cmd_id`: telemetry command identifier
- `payload_len`: expected payload size in bytes
- `fields`: ordered list of `name` + `type` entries (for byte layout)

Optional per-field metadata:

- `scale`: numeric multiplier applied to raw field value
- `offset`: numeric offset added after scaling
- `helper`: generated C++ helper method name returning `double`
- `enum_type`: references a TM enum collection key (for example `gimbal_working_modes`)
- `enum_helper`: generated C++ helper method name returning the enum type
- `unit`: descriptive unit string for schema readability (not used in code generation)

Supported TM field types in the generator are: `uint8`, `int8`, `uint16_le`, `int16_le`, `uint32_le`, `int32_le`, `uint64_le`, `int64_le`.

The following headers are generated from these YAML files:

- `include/camera/protocol/tc_parameter.hpp`
- `include/camera/protocol/tm_parameters.hpp`

Do not edit those generated headers manually.

Regenerate with script:

```bash
./scripts/generate_protocol_headers.py
```

Or via CMake target:

```bash
cmake --build --preset debug --target generate_protocol_headers
```

## Demo App Usage

Build:

```bash
mkdir -p build && cd build
cmake ..
cmake --build .
```

Run examples:

```bash
./siyi_demo 192.168.144.25 37260 acquire-fw-ver
./siyi_demo 192.168.144.25 37260 set-utc-time 1715072000000000
./siyi_demo 192.168.144.25 37260 auto-focus 1 2
./siyi_demo 192.168.144.25 37260 soft-restart 1 0
```

## Integration Tests with Real Camera

This section is for end-to-end checks against a physical SIYI camera.

### Direct connection

If your machine is directly connected to the camera network:

```bash
./build/debug/siyi_demo 192.168.144.25 37260 acquire-fw-ver
./build/debug/siyi_demo 192.168.144.25 37260 picture
./build/debug/siyi_demo 192.168.144.25 37260 acquire-gimbal-att --timeout-ms 1500
```

### Remote routing through another PC

If your development PC is not directly connected to the camera, route traffic through an intermediate PC.

Example addressing:
- camera: `192.168.144.25`
- bridge PC (connected to camera): `192.168.1.14`
- development PC: `192.168.1.30`

On development PC:

```bash
sudo ip route add 192.168.144.0/24 via 192.168.1.14
ping 192.168.144.25
```

On bridge PC:

```bash
# Example interface names only, adapt to your machine.
sudo iptables -A FORWARD -i wlP1p1s0 -o enP8p1s0 -j ACCEPT
sudo iptables -A FORWARD -i enP8p1s0 -o wlP1p1s0 -m state --state ESTABLISHED,RELATED -j ACCEPT
sudo iptables -t nat -A POSTROUTING -o enP8p1s0 -j MASQUERADE
sudo sysctl -w net.ipv4.ip_forward=1
sudo tcpdump -i any port 37260
```

After routing is set, validate behavior with a feedback-producing command such as `picture` or `acquire-gimbal-att`.

## Tests

```bash
bash scripts/run_tests.sh
```

or with presets:

```bash
cmake --preset debug
cmake --build --preset debug
ctest --preset debug
```

## Export TC/TM Matrix for Release

Generate release artifacts:

```bash
bash scripts/export_tc_tm.sh
```

Outputs:
- `release/tc_tm_matrix.md`
- `release/tc_tm_matrix.csv`

These files summarize currently implemented telecommands and typed telemetry decode support.
