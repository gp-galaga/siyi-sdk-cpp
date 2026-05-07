#!/usr/bin/env python3
"""Generate protocol headers from YAML definitions.

This script updates:
    - include/camera/enum/cmd_parameter.hpp (from protocol/tc/definitions.yaml)
    - include/camera/enum/tm_parameters.hpp  (from protocol/tm/definitions.yaml)
"""

from __future__ import annotations

import argparse
from pathlib import Path
from typing import Iterable


TM_TYPE_TO_CPP = {
    "uint8": "uint8_t",
    "int8": "int8_t",
    "uint16_le": "uint16_t",
    "int16_le": "int16_t",
    "uint32_le": "uint32_t",
    "int32_le": "int32_t",
    "uint64_le": "uint64_t",
    "int64_le": "int64_t",
}

TM_TYPE_TO_BYTES = {
    "uint8": 1,
    "int8": 1,
    "uint16_le": 2,
    "int16_le": 2,
    "uint32_le": 4,
    "int32_le": 4,
    "uint64_le": 8,
    "int64_le": 8,
}


def _format_uint8_literal(value: int) -> str:
    if value < 0:
        raise ValueError("uint8 literal cannot be negative")
    return f"0x{value:02X}"


def _enum_lines(items: Iterable[dict], *, hex_values: bool = False, indent: int = 8) -> list[str]:
    rows = list(items)
    out: list[str] = []
    for index, row in enumerate(rows):
        value = int(row["value"], 0) if isinstance(row["value"], str) else int(row["value"])
        if hex_values:
            value_text = _format_uint8_literal(value)
        else:
            value_text = str(value)
        trailing = "," if index < len(rows) - 1 else ""
        out.append(f"{' ' * indent}{row['name']} = {value_text}{trailing}")
    return out


def _snake_to_pascal(name: str) -> str:
    parts = [part for part in name.split("_") if part]
    return "".join(part[:1].upper() + part[1:] for part in parts)


def _default_tm_enum_cpp_name(enum_key: str) -> str:
    # Preserve historical names for backward compatibility.
    if enum_key == "command_acknowledgement":
        return "COMMAND_ACKNOLEDGEMENT"
    if enum_key == "gimbal_models":
        return "GimbalModel"

    base_key = enum_key[:-1] if enum_key.endswith("s") else enum_key
    return _snake_to_pascal(base_key)


def _render_cmd_parameter(tc_data: dict) -> str:
    control = _enum_lines(tc_data["control_flags"], hex_values=True, indent=8)
    commands = _enum_lines(tc_data["commands"], hex_values=True, indent=8)
    photo = _enum_lines(tc_data["photo_record_functions"], indent=8)
    zoom_dir = _enum_lines(tc_data["manual_zoom_directions"], indent=8)

    camera_models = tc_data.get("camera_models", [])
    camera_model_lines: list[str] = []
    for index, model in enumerate(camera_models):
        trailing = "," if index < len(camera_models) - 1 else ""
        camera_model_lines.append(f"        {model['name']}{trailing}")

    metadata_rows = tc_data.get("command_metadata", [])
    scope_by_cmd: dict[str, str] = {}
    supported_by_cmd: dict[str, list[str]] = {}
    for row in metadata_rows:
        cmd = str(row["cmd"])
        scope_by_cmd[cmd] = str(row.get("scope", "COMMON"))
        supported_models = [str(model) for model in row.get("supported_models", [])]
        supported_by_cmd[cmd] = supported_models

    scope_switch: list[str] = []
    support_switch: list[str] = []
    for cmd in tc_data["commands"]:
        cmd_name = str(cmd["name"])
        scope = scope_by_cmd.get(cmd_name, "COMMON")
        scope_switch.append(f"            case CommandId::{cmd_name}: return CommandScope::{scope};")

        supported_models = supported_by_cmd.get(cmd_name)
        if not supported_models:
            support_switch.append(f"            case CommandId::{cmd_name}: return true;")
            continue

        support_switch.append(f"            case CommandId::{cmd_name}:")
        support_switch.append("                switch (model)")
        support_switch.append("                {")
        for model in supported_models:
            support_switch.append(f"                    case CameraModel::{model}: return true;")
        support_switch.append("                    default: return false;")
        support_switch.append("                }")

    return "\n".join(
        [
            "#ifndef SIYI_CAMERA_CMD_PARAMETER_HPP",
            "#define SIYI_CAMERA_CMD_PARAMETER_HPP",
            "",
            "#include <cstdint>",
            "",
            "namespace SIYI",
            "{",
            "    // AUTO-GENERATED FILE. DO NOT EDIT DIRECTLY.",
            "    // Source: protocol/tc/definitions.yaml",
            "",
            "    // Control byte values (SIYI wire protocol):",
            "    //   0x00  host -> device, no ACK requested",
            "    //   0x01  host -> device, ACK requested",
            "    //   0x02  device -> host, ACK response",
            "    enum class ControlFlag : uint8_t",
            "    {",
            *control,
            "    };",
            "",
            "    inline uint8_t MakeHostRequestControlByte() noexcept",
            "    {",
            "        return static_cast<uint8_t>(ControlFlag::NEED_ACK);",
            "    }",
            "",
            "    inline uint8_t MakeDeviceAckControlByte() noexcept",
            "    {",
            "        return static_cast<uint8_t>(ControlFlag::ACK);",
            "    }",
            "",
            "    enum class CommandId : uint8_t",
            "    {",
            *commands,
            "    };",
            "",
            "    enum class PhotoRecordFunction : uint8_t",
            "    {",
            *photo,
            "    };",
            "",
            "    enum class ManualZoomDirection : int8_t",
            "    {",
            *zoom_dir,
            "    };",
            "",
            "    enum class CommandScope : uint8_t",
            "    {",
            "        COMMON = 0,",
            "        ZOOM_CAMERA = 1",
            "    };",
            "",
            "    enum class CameraModel : uint8_t",
            "    {",
            *camera_model_lines,
            "    };",
            "",
            "    inline CommandScope GetCommandScope(CommandId cmd) noexcept",
            "    {",
            "        switch (cmd)",
            "        {",
            *scope_switch,
            "            default: return CommandScope::COMMON;",
            "        }",
            "    }",
            "",
            "    inline bool IsCommandSupportedByCamera(CommandId cmd, CameraModel model) noexcept",
            "    {",
            "        switch (cmd)",
            "        {",
            *support_switch,
            "            default: return true;",
            "        }",
            "    }",
            "}",
            "",
            "#endif",
            "",
        ]
    )


def _message_fields_to_lines(message: dict) -> list[str]:
    lines: list[str] = []
    for field in message["fields"]:
        field_type = field["type"]
        cpp_type = TM_TYPE_TO_CPP.get(field_type)
        if cpp_type is None:
            raise ValueError(f"Unknown TM field type: {field_type}")
        lines.append(f"            {cpp_type} {field['name']};")
    return lines


def _as_float_literal(value: object) -> str:
    if isinstance(value, (int, float)):
        return repr(float(value))
    return repr(float(str(value)))


def _default_helper_name(field_name: str) -> str:
    return f"{field_name[0].upper()}{field_name[1:]}Scaled"


def _default_enum_helper_name(field_name: str) -> str:
    return f"As{field_name[0].upper()}{field_name[1:]}"


def _field_helpers_to_lines(message: dict, enum_name_by_key: dict[str, str]) -> list[str]:
    lines: list[str] = []
    for field in message["fields"]:
        enum_key = field.get("enum_type")
        if enum_key is not None:
            enum_cpp_name = enum_name_by_key.get(str(enum_key))
            if enum_cpp_name is None:
                raise ValueError(f"Unknown enum_type '{enum_key}' in message '{message['name']}' field '{field['name']}'")
            helper_name = field.get("enum_helper") or field.get("helper") or _default_enum_helper_name(field["name"])
            lines.append(
                f"            {enum_cpp_name} {helper_name}() const {{ return static_cast<{enum_cpp_name}>({field['name']}); }}"
            )
            continue

        has_scale = "scale" in field
        has_offset = "offset" in field
        helper_name = field.get("helper")

        if not has_scale and not has_offset and not helper_name:
            continue

        if helper_name is None:
            helper_name = _default_helper_name(field["name"])

        expression = f"static_cast<double>({field['name']})"
        if has_scale:
            expression = f"({expression} * {_as_float_literal(field['scale'])})"
        if has_offset:
            expression = f"({expression} + {_as_float_literal(field['offset'])})"

        lines.append(f"            double {helper_name}() const {{ return {expression}; }}")
    return lines


def _validate_tm_message(message: dict) -> None:
    payload_len = int(message["payload_len"])
    fields = message.get("fields", [])
    computed_len = 0
    for field in fields:
        field_type = field["type"]
        byte_len = TM_TYPE_TO_BYTES.get(field_type)
        if byte_len is None:
            raise ValueError(f"Unknown TM field type: {field_type}")
        computed_len += byte_len
    if computed_len != payload_len:
        raise ValueError(
            f"TM message '{message['name']}' payload_len mismatch: declared={payload_len}, computed={computed_len}"
        )

    for field in fields:
        if "scale" in field:
            _ = _as_float_literal(field["scale"])
        if "offset" in field:
            _ = _as_float_literal(field["offset"])


def _collect_tm_enums(tm_data: dict, message_names: set[str]) -> tuple[dict[str, list[dict]], dict[str, str]]:
    enum_data: dict[str, list[dict]] = {}
    enum_name_by_key: dict[str, str] = {}
    for key, value in tm_data.items():
        if key == "typed_messages":
            continue
        if not isinstance(value, list):
            continue

        rows = value
        if not rows:
            continue
        if not all(isinstance(item, dict) and "name" in item and "value" in item for item in rows):
            continue

        enum_data[key] = rows
        enum_cpp_name = _default_tm_enum_cpp_name(key)
        if enum_cpp_name in message_names:
            enum_cpp_name = f"{enum_cpp_name}Enum"
        enum_name_by_key[key] = enum_cpp_name
    return enum_data, enum_name_by_key


def _render_tm_parameter(tm_data: dict) -> str:
    typed_messages = tm_data.get("typed_messages", [])
    message_names = {str(message["name"]) for message in typed_messages}

    enum_data, enum_name_by_key = _collect_tm_enums(tm_data, message_names)

    enum_blocks: list[str] = []
    for enum_key, rows in enum_data.items():
        enum_cpp_name = enum_name_by_key[enum_key]
        enum_lines = _enum_lines(rows, hex_values=any(isinstance(row["value"], str) and str(row["value"]).lower().startswith("0x") for row in rows), indent=12)
        enum_blocks.append(f"        enum class {enum_cpp_name} : uint8_t")
        enum_blocks.append("        {")
        enum_blocks.extend(enum_lines)
        enum_blocks.append("        };")
        enum_blocks.append("")

    alias_blocks: list[str] = []
    for message in typed_messages:
        _validate_tm_message(message)
        message_name = str(message["name"])

    struct_blocks: list[str] = []
    for message in typed_messages:
        struct_blocks.append(f"        struct {message['name']}")
        struct_blocks.append("        {")
        struct_blocks.extend(_message_fields_to_lines(message))

        helper_lines = _field_helpers_to_lines(message, enum_name_by_key)
        if helper_lines:
            struct_blocks.append("")
            struct_blocks.extend(helper_lines)

        struct_blocks.append("        };")
        struct_blocks.append("")

    variant_types = ", ".join(message["name"] for message in typed_messages)

    return "\n".join(
        [
            "#ifndef __SIYI_CAMERA_TM_HPP__",
            "#define __SIYI_CAMERA_TM_HPP__",
            "",
            "#include <cstdint>",
            "#include <variant>",
            "",
            "namespace SIYI",
            "{",
            "    namespace TM",
            "    {",
            "        // AUTO-GENERATED FILE. Message structs and enums come from YAML.",
            "        // Source: protocol/tm/definitions.yaml",
            "",
            *enum_blocks,
            *struct_blocks,
            *alias_blocks,
            *([""] if alias_blocks else []),
            f"        using TelemetryMessage = std::variant<{variant_types}>;",
            "    };",
            "}",
            "",
            "#endif // __SIYI_CAMERA_TM_HPP__",
            "",
        ]
    )


def _load_yaml(yaml_path: Path) -> dict:
    try:
        import yaml  # type: ignore
    except Exception as exc:  # pragma: no cover
        raise SystemExit(
            "PyYAML is required to generate headers. Install with: pip install pyyaml"
        ) from exc

    with yaml_path.open("r", encoding="utf-8") as handle:
        return yaml.safe_load(handle)


def main() -> int:
    parser = argparse.ArgumentParser(description="Generate protocol headers from YAML")
    parser.add_argument(
        "--root",
        default=str(Path(__file__).resolve().parent.parent),
        help="Project root directory",
    )
    args = parser.parse_args()

    root = Path(args.root).resolve()
    tc_yaml_path = root / "protocol" / "tc" / "definitions.yaml"
    tm_yaml_path = root / "protocol" / "tm" / "definitions.yaml"
    out_cmd = root / "include" / "camera" / "enum" / "tc_parameter.hpp"
    out_tm = root / "include" / "camera" / "enum" / "tm_parameters.hpp"

    tc_data = _load_yaml(tc_yaml_path)
    tm_data = _load_yaml(tm_yaml_path)

    out_cmd.write_text(_render_cmd_parameter(tc_data), encoding="utf-8")
    out_tm.write_text(_render_tm_parameter(tm_data), encoding="utf-8")

    print(f"Generated: {out_cmd}")
    print(f"Generated: {out_tm}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())