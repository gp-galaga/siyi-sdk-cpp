#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
OUT_DIR="$ROOT_DIR/release"
MD_OUT="$OUT_DIR/tc_tm_matrix.md"
CSV_OUT="$OUT_DIR/tc_tm_matrix.csv"

mkdir -p "$OUT_DIR"

DATE_UTC="$(date -u +"%Y-%m-%dT%H:%M:%SZ")"

cat > "$MD_OUT" <<EOF
# SIYI TC/TM Matrix

Generated at: $DATE_UTC (UTC)

## Telecommands (TC)

| CLI Command | API Method | CMD_ID | Payload | ACK |
|---|---|---:|---|---|
| acquire-fw-ver | AcquireFirmwareVersion() | 0x01 | empty | yes |
| acquire-hw-id | AcquireHardwareId() | 0x02 | empty | yes |
| auto-focus [x] [y] | AutoFocus(x, y) | 0x04 | 0x01 + x(2B) + y(2B) | yes |
| zoom <speed> | SetAbsoluteZoom(int8_t speed, needAck) | 0x05 | int8 speed | configurable |
| rotate <yaw> <pitch> | StartRotation(yaw, pitch, needAck) | 0x07 | 2 bytes speed | configurable |
| stop-rotation | StopRotation(needAck) | 0x07 | 0x00 0x00 | configurable |
| center | Center() | 0x08 | 0x01 | yes |
| acquire-gimbal-info | AcquireGimbalConfiguration() | 0x0A | empty | yes |
| acquire-gimbal-att | AcquireGimbalAttitude() | 0x0D | empty | yes |
| absolute-zoom <value> | SetAbsoluteZoom(float) | 0x0F | int, frac*10 | yes |
| set-utc-time <uint64> | SetUtcTime(uint64_t us) | 0x30 | uint64 LE (8B) | yes |
| soft-restart [cam] [gimbal] | SoftRestart(cam, gimbal) | 0x80 | 2 flags | yes |
| picture | TakePicture() | 0x0C | 0 | no |
| record | StartStopRecording() | 0x0C | 2 | no |
| hdr | ToggleHDR() | 0x0C | 1 | no |
| lock | ControlPhotoRecord(MOTION_LOCK_MODE) | 0x0C | 3 | no |
| follow | ControlPhotoRecord(MOTION_FOLLOW_MODE) | 0x0C | 4 | no |
| fpv | ControlPhotoRecord(MOTION_FPV_MODE) | 0x0C | 5 | no |
| video-hdmi | ControlPhotoRecord(VIDEO_OUTPUT_HDMI) | 0x0C | 6 | no |
| video-cvbs | ControlPhotoRecord(VIDEO_OUTPUT_CVBS) | 0x0C | 7 | no |
| video-off | ControlPhotoRecord(VIDEO_OUTPUT_OFF) | 0x0C | 8 | no |

## Typed Telemetry (TM)

| CMD_ID | Typed Message | Payload Length |
|---:|---|---:|
| 0x0A | TM::GimbalConfigurationTM | 7 bytes |
| 0x0D | TM::GimbalAttitudeTM | 12 bytes |
EOF

cat > "$CSV_OUT" <<EOF
kind,cli_or_source,api_or_message,cmd_id,payload,ack_or_len
TC,acquire-fw-ver,AcquireFirmwareVersion(),0x01,empty,yes
TC,acquire-hw-id,AcquireHardwareId(),0x02,empty,yes
TC,auto-focus [x] [y],AutoFocus(x\, y),0x04,0x01 + x(2B) + y(2B),yes
TC,zoom <speed>,SetAbsoluteZoom(int8_t speed\, needAck),0x05,int8 speed,configurable
TC,rotate <yaw> <pitch>,StartRotation(yaw\, pitch\, needAck),0x07,2 bytes speed,configurable
TC,stop-rotation,StopRotation(needAck),0x07,0x00 0x00,configurable
TC,center,Center(),0x08,0x01,yes
TC,acquire-gimbal-info,AcquireGimbalConfiguration(),0x0A,empty,yes
TC,acquire-gimbal-att,AcquireGimbalAttitude(),0x0D,empty,yes
TC,absolute-zoom <value>,SetAbsoluteZoom(float),0x0F,int\, frac*10,yes
TC,set-utc-time <uint64>,SetUtcTime(uint64_t us),0x30,uint64 LE (8B),yes
TC,soft-restart [cam] [gimbal],SoftRestart(cam\, gimbal),0x80,2 flags,yes
TC,picture,TakePicture(),0x0C,0,no
TC,record,StartStopRecording(),0x0C,2,no
TC,hdr,ToggleHDR(),0x0C,1,no
TC,lock,ControlPhotoRecord(MOTION_LOCK_MODE),0x0C,3,no
TC,follow,ControlPhotoRecord(MOTION_FOLLOW_MODE),0x0C,4,no
TC,fpv,ControlPhotoRecord(MOTION_FPV_MODE),0x0C,5,no
TC,video-hdmi,ControlPhotoRecord(VIDEO_OUTPUT_HDMI),0x0C,6,no
TC,video-cvbs,ControlPhotoRecord(VIDEO_OUTPUT_CVBS),0x0C,7,no
TC,video-off,ControlPhotoRecord(VIDEO_OUTPUT_OFF),0x0C,8,no
TM,DecodeTelemetryPacket,TM::GimbalConfigurationTM,0x0A,n/a,7 bytes
TM,DecodeTelemetryPacket,TM::GimbalAttitudeTM,0x0D,n/a,12 bytes
EOF

echo "Generated: $MD_OUT"
echo "Generated: $CSV_OUT"
