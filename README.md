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

## Project Layout

- `protocol/tc/definitions.yaml`: source-of-truth for telecommand (TC) enums and IDs
- `protocol/tm/definitions.yaml`: source-of-truth for telemetry (TM) enums, message IDs, and typed payload fields
- `include/camera/enum/cmd_parameter.hpp`: command IDs and control flags
- `include/camera/enum/tm_parameters.hpp`: typed telemetry models
- `include/camera/icamera_tc.hpp`: camera/session interface
- `include/camera/ibase_camera.hpp`: base SIYI camera command helpers
- `include/camera/izr_camera.hpp`: ZR30 class
- `src/camera/base_camera.cpp`: packet codec + base logic
- `src/camera/zr30_camera.cpp`: ZR30 model setup
- `apps/main.cpp`: UDP demo app
- `tests/test_siyi_commands.cpp`: unit tests
- `scripts/run_tests.sh`: test helper script
- `scripts/generate_protocol_headers.py`: generates enum headers from YAML
- `scripts/export_tc_tm.sh`: export TC/TM matrix for release

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

- `include/camera/enum/tc_parameter.hpp`
- `include/camera/enum/tm_parameters.hpp`

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
./out/build/debug/siyi_demo 192.168.144.25 37260 acquire-fw-ver
./out/build/debug/siyi_demo 192.168.144.25 37260 picture
./out/build/debug/siyi_demo 192.168.144.25 37260 acquire-gimbal-att --timeout-ms 1500
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
