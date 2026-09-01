#include "../include/siyi/camera/managers/camera_manager.hpp"
#include "../include/siyi/camera/models/optical_zoom_camera_model.hpp"

#include <cerrno>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <thread>
#include <chrono>
#include <vector>

namespace
{
    void PrintUsage(const char* programName)
    {
        std::cerr
            << "Usage:\n"
            << "  " << programName << " <ip> <port> <command> [args] [--transport udp|tcp] [--no-ack] [--timeout-ms N] [--bind-port P] [--repeat N] [--period-ms N]\n\n"
            << "Commands:\n"
            << "  picture\n"
            << "  record\n"
            << "  hdr\n"
            << "  tcp-heartbeat\n"
            << "  zoom <speed>\n"
            << "  rotate <yaw_speed> <pitch_speed>\n"
            << "  stop-rotation\n"
            << "  acquire-gimbal-att\n"
            << "  acquire-gimbal-info\n"
            << "  acquire-fw-ver\n"
            << "  acquire-hw-id\n"
            << "  auto-focus [x_coord] [y_coord]\n"
            << "  manual-focus <direction>\n"
            << "  center\n"
            << "  absolute-zoom <value>\n"
            << "  set-gimbal-angle <yaw-degrees> <pitch-degrees>\n"
            << "  set-utc-time <uint64>\n"
            << "  soft-restart [camera_reboot:0|1] [gimbal_reset:0|1]\n" 
            << "  lock\n"
            << "  follow\n"
            << "  fpv\n"
            << "  video-hdmi\n"
            << "  video-cvbs\n"
            << "  video-off\n"
            << "  feedback-info\n"
            << "  get-zoom-level\n"
            // << "  zr-follow <0|1>\n\n"
            << "Examples:\n"
            << "  " << programName << " 192.168.144.25 37260 picture\n"
            << "  " << programName << " 192.168.144.25 37260 zoom 1\n"
            << "  " << programName << " 192.168.144.25 37260 rotate 100 100 --repeat 30 --period-ms 100\n";
    }

    bool ParseInt8(const std::string& value, int8_t& outValue)
    {
        char* end = nullptr;
        errno = 0;
        const long parsed = std::strtol(value.c_str(), &end, 10);
        if (errno != 0 || end == value.c_str() || *end != '\0' || parsed < -128 || parsed > 127)
        {
            return false;
        }
        outValue = static_cast<int8_t>(parsed);
        return true;
    }

    bool ParseUInt16(const std::string& value, uint16_t& outValue)
    {
        char* end = nullptr;
        errno = 0;
        const unsigned long parsed = std::strtoul(value.c_str(), &end, 10);
        if (errno != 0 || end == value.c_str() || *end != '\0' || parsed > 65535UL)
        {
            return false;
        }
        outValue = static_cast<uint16_t>(parsed);
        return true;
    }

    bool ParseInt16(const std::string& value, int16_t& outValue)
    {
        char* end = nullptr;
        errno = 0;
        const long parsed = std::strtol(value.c_str(), &end, 10);
        if (errno != 0 || end == value.c_str() || *end != '\0' || parsed < -32768L || parsed > 32767L)
        {
            return false;
        }
        outValue = static_cast<int16_t>(parsed);
        return true;
    }

    bool ParsePositiveInt(const std::string& value, int& outValue, int minValue, int maxValue)
    {
        char* end = nullptr;
        errno = 0;
        const long parsed = std::strtol(value.c_str(), &end, 10);
        if (errno != 0 || end == value.c_str() || *end != '\0' || parsed < minValue || parsed > maxValue)
        {
            return false;
        }
        outValue = static_cast<int>(parsed);
        return true;
    }

    bool ParseUInt64(const std::string& value, uint64_t& outValue)
    {
        char* end = nullptr;
        errno = 0;

        const unsigned long long parsed = std::strtoull(value.c_str(), &end, 10);
        if (errno != 0 || end == value.c_str() || *end != '\0')
        {
            return false;
        }
        outValue = static_cast<uint64_t>(parsed);
        return true;
    }

    bool ParseFloat(const std::string& value, float& outValue)
    {
        char* end = nullptr;
        errno = 0;
        const float parsed = std::strtof(value.c_str(), &end);
        if (errno != 0 || end == value.c_str() || *end != '\0')
        {
            return false;
        }
        outValue = parsed;
        return true;
    }

    std::optional<std::vector<uint8_t>> BuildCommand(
        SIYI::SharedCameraModel& camera,
        const std::vector<std::string>& args,
        const bool needAck)
    {
        if (args.empty())
        {
            return std::nullopt;
        }

        const std::string& command = args[0];
        auto* opticalCamera = dynamic_cast<SIYI::OpticalZoomCameraModel*>(&camera);
        auto requireOptical = [&](const char* cmdName) -> SIYI::OpticalZoomCameraModel* {
            if (opticalCamera == nullptr)
            {
                std::cerr << cmdName << " requires an optical-zoom camera model\n";
                return nullptr;
            }
            return opticalCamera;
        };

        if (command == "stop-rotation")
        {
            return camera.StopRotation();
        }
        if (command == "acquire-gimbal-att")
        {
            return camera.AcquireGimbalAttitude();
        }
        if (command == "acquire-gimbal-info")
        {
            return camera.AcquireGimbalConfiguration();
        }
        if (command == "acquire-fw-ver")
        {
            return camera.AcquireFirmwareVersion();
        }
        if (command == "acquire-hw-id")
        {
            return camera.AcquireHardwareId();
        }
        if (command == "auto-focus")
        {
            auto* zoomCamera = requireOptical("auto-focus");
            if (zoomCamera == nullptr)
            {
                return std::nullopt;
            }

            uint16_t xCoord = 100;
            uint16_t yCoord = 200;

            if (args.size() >= 2 && !ParseUInt16(args[1], xCoord))
            {
                std::cerr << "invalid auto-focus x_coord: " << args[1] << "\n";
                return std::nullopt;
            }
            if (args.size() >= 3 && !ParseUInt16(args[2], yCoord))
            {
                std::cerr << "invalid auto-focus y_coord: " << args[2] << "\n";
                return std::nullopt;
            }
            return zoomCamera->AutoFocus(xCoord, yCoord);
        }
        if(command == "manual-focus"){
            auto* zoomCamera = requireOptical("manual-focus");
            if (zoomCamera == nullptr)
            {
                return std::nullopt;
            }

            if (args.size() < 2)
            {
                std::cerr << "manual-focus requires one argument: <direction>\n";
                return std::nullopt;
            }
            const std::string& directionStr = args[1];
            SIYI::ManualFocusDirection direction;
            if (directionStr == "stop" || directionStr == "0")
            {
                direction = SIYI::ManualFocusDirection::STOP;
            }
            else if (directionStr == "long-shot" || directionStr == "1")
            {
                direction = SIYI::ManualFocusDirection::LONG_SHOT;
            }
            else if (directionStr == "close-shot" || directionStr == "-1")
            {
                direction = SIYI::ManualFocusDirection::CLOSE_SHOT;
            }
            else
            {
                std::cerr << "invalid manual-focus direction: " << directionStr << " (expected 'stop' or '0', 'long-shot' or '1', or 'close-shot' or '-1')\n";
                return std::nullopt;
            }
            return zoomCamera->SetManualFocus(direction);
        }
        if (command == "center")
        {
            return camera.Center();
        }
        if (command == "picture")
        {
            return camera.TakePicture();
        }
        if (command == "record")
        {
            return camera.StartStopRecording();
        }
        if (command == "hdr")
        {
            return camera.ToggleHDR();
        }
        if (command == "tcp-heartbeat")
        {
            return camera.BuildCustomCommand(
                static_cast<uint8_t>(SIYI::CommandId::TCP_HEARTBEAT),
                {0x00},
                true);
        }
        if (command == "lock")
        {
            return camera.ControlPhotoRecord(SIYI::PhotoRecordFunction::MOTION_LOCK_MODE);
        }
        // problème follow
        if (command == "follow")
        {
            return camera.ControlPhotoRecord(SIYI::PhotoRecordFunction::MOTION_FOLLOW_MODE);
        }
        if (command == "fpv")
        {
            return camera.ControlPhotoRecord(SIYI::PhotoRecordFunction::MOTION_FPV_MODE);
        }
        if (command == "video-hdmi")
        {
            return camera.ControlPhotoRecord(SIYI::PhotoRecordFunction::VIDEO_OUTPUT_HDMI);
        }
        if (command == "video-cvbs")
        {
            return camera.ControlPhotoRecord(SIYI::PhotoRecordFunction::VIDEO_OUTPUT_CVBS);
        }
        if (command == "video-off")
        {
            return camera.ControlPhotoRecord(SIYI::PhotoRecordFunction::VIDEO_OUTPUT_OFF);
        }
        if (command == "zoom")
        {
            auto* zoomCamera = requireOptical("zoom");
            if (zoomCamera == nullptr)
            {
                return std::nullopt;
            }

            if (args.size() < 2)
            {
                std::cerr << "zoom requires one argument: <direction> (stop|0, in|1, out|-1)\n";
                return std::nullopt;
            }
            int8_t zoomSpeed = 0;
            if (!ParseInt8(args[1], zoomSpeed))
            {
                std::cerr << "invalid zoom direction: " << args[1] << "\n";
                return std::nullopt;
            }

            SIYI::ManualZoomDirection direction = SIYI::ManualZoomDirection::STOP;
            if (zoomSpeed > 0)
            {
                direction = SIYI::ManualZoomDirection::ZOOM_IN;
            }
            else if (zoomSpeed < 0)
            {
                direction = SIYI::ManualZoomDirection::ZOOM_OUT;
            }

            return zoomCamera->SetManualZoom(direction);
        }
        if (command == "absolute-zoom")
        {
            auto* zoomCamera = requireOptical("absolute-zoom");
            if (zoomCamera == nullptr)
            {
                return std::nullopt;
            }

            if (args.size() < 2)
            {
                std::cerr << "absolute-zoom requires one argument: <value>\n";
                return std::nullopt;
            }

            float zoomValue = 0.0F;
            if (!ParseFloat(args[1], zoomValue) || zoomValue < 0.5F || zoomValue > 180.0F)
            {
                std::cerr << "invalid absolute-zoom value: " << args[1] << " (expected range 1.0 to 180.0)\n";
                return std::nullopt;
            }

            // const float scaled = zoomValue * 10.0F;
            // const int scaledInt = static_cast<int>(scaled + 0.5F);
            // if (std::abs(scaled - static_cast<float>(scaledInt)) > 0.001F)
            // {
            //     std::cerr << "invalid absolute-zoom value: " << args[1] << " (expected a single decimal digit)\n";
            //     return std::nullopt;
            // }

            const int int_part = static_cast<int>(zoomValue);
            const int frac_part = static_cast<int>((zoomValue - static_cast<float>(int_part)) * 10.0F + 0.5F);
            if (frac_part > 9)
            {
                std::cerr << "invalid absolute-zoom value: " << args[1] << " (fractional part too large after scaling)\n";
                return std::nullopt;
            }

            return zoomCamera->SetAbsoluteZoom(zoomValue);
        }
        if (command == "set-gimbal-angle")
        {
            if (args.size() < 3)
            {
                std::cerr << "set-gimbal-angle requires two arguments: <yaw> <pitch>\n";
                return std::nullopt;
            }

            int16_t yaw = 0;
            int16_t pitch = 0;
            if (!ParseInt16(args[1], yaw) || !ParseInt16(args[2], pitch))
            {
                std::cerr << "invalid set-gimbal-angle arguments\n";
                return std::nullopt;
            }

            return camera.SetGimbalAngle(yaw, pitch);
        }
        if (command == "set-utc-time")
        {
            if (args.size() < 2)
            {
                std::cerr << "set-utc-time requires one argument: <uint64>\n";
                return std::nullopt;
            }

            uint64_t utcTime = 0;
            if (!ParseUInt64(args[1], utcTime))
            {
                std::cerr << "invalid set-utc-time value: " << args[1] << "\n";
                return std::nullopt;
            }

            return camera.SetUtcTime(utcTime);
        }
        if (command == "soft-restart")
        {
            uint8_t cameraReboot = 0;
            uint8_t gimbalReset = 0;

            if (args.size() >= 2)
            {
                int parsed = 0;
                if (!ParsePositiveInt(args[1], parsed, 0, 1))
                {
                    std::cerr << "invalid camera_reboot value: " << args[1] << " (expected 0 or 1)\n";
                    return std::nullopt;
                }
                cameraReboot = static_cast<uint8_t>(parsed);
            }

            if (args.size() >= 3)
            {
                int parsed = 0;
                if (!ParsePositiveInt(args[2], parsed, 0, 1))
                {
                    std::cerr << "invalid gimbal_reset value: " << args[2] << " (expected 0 or 1)\n";
                    return std::nullopt;
                }
                gimbalReset = static_cast<uint8_t>(parsed);
            }

            return camera.SoftRestart(cameraReboot, gimbalReset);
        }
        if (command == "rotate")
        {
            if (args.size() < 3)
            {
                std::cerr << "rotate requires two arguments: <yaw_speed> <pitch_speed>\n";
                return std::nullopt;
            }
            int8_t yawSpeed = 0;
            int8_t pitchSpeed = 0;
            if (!ParseInt8(args[1], yawSpeed) || !ParseInt8(args[2], pitchSpeed))
            {
                std::cerr << "invalid rotate arguments\n";
                return std::nullopt;
            }
            return camera.StartRotation(yawSpeed, pitchSpeed);
        }
        if(command == "feedback-info")
        {
            return camera.AcquireFunctionFeedbackInfo();
        }
        if(command == "get-zoom-level")
        {
            auto* zoomCamera = requireOptical("get-zoom-level");
            if (zoomCamera == nullptr)
            {
                return std::nullopt;
            }
            return zoomCamera->AcquireZoomLevel();
        }


        std::cerr << "unknown command: " << command << "\n";
        return std::nullopt;
    }

    bool CommandExpectedToAck(const std::vector<std::string>& args)
    {
        if (args.empty())
        {
            return false;
        }
        return true;
    }

}

int main(int argc, char** argv)
{
    if (argc < 4)
    {
        PrintUsage(argv[0]);
        return 1;
    }

    const std::string ipAddress = argv[1];
    uint16_t port = 0;
    if (!ParseUInt16(argv[2], port))
    {
        std::cerr << "invalid port: " << argv[2] << "\n";
        return 1;
    }

    bool needAck = true;
    int timeoutMs = 1000;
    int repeatCount = 1;
    int periodMs = 100;
    uint16_t bindPort = 0;
    std::string transportName = "udp";
    std::vector<std::string> commandArgs;
    for (int i = 3; i < argc; ++i)
    {
        const std::string arg = argv[i];
        if (arg == "--transport")
        {
            if (i + 1 >= argc)
            {
                std::cerr << "--transport requires a value (udp or tcp)\n";
                return 1;
            }
            transportName = argv[++i];
            if (transportName != "udp" && transportName != "tcp")
            {
                std::cerr << "invalid --transport value: " << transportName << " (expected udp or tcp)\n";
                return 1;
            }
            continue;
        }
        if (arg == "--no-ack")
        {
            needAck = false;
            continue;
        }
        if (arg == "--timeout-ms")
        {
            if (i + 1 >= argc)
            {
                std::cerr << "--timeout-ms requires a value\n";
                return 1;
            }
            char* end = nullptr;
            const long parsed = std::strtol(argv[++i], &end, 10);
            if (end == argv[i] || *end != '\0' || parsed < 0 || parsed > 60000)
            {
                std::cerr << "invalid timeout: " << argv[i] << "\n";
                return 1;
            }
            timeoutMs = static_cast<int>(parsed);
            continue;
        }
        if (arg == "--repeat")
        {
            if (i + 1 >= argc || !ParsePositiveInt(argv[++i], repeatCount, 1, 10000))
            {
                std::cerr << "invalid --repeat value\n";
                return 1;
            }
            continue;
        }
        if (arg == "--period-ms")
        {
            if (i + 1 >= argc || !ParsePositiveInt(argv[++i], periodMs, 0, 60000))
            {
                std::cerr << "invalid --period-ms value\n";
                return 1;
            }
            continue;
        }
        if (arg == "--bind-port")
        {
            if (i + 1 >= argc || !ParseUInt16(argv[++i], bindPort))
            {
                std::cerr << "invalid --bind-port value\n";
                return 1;
            }
            continue;
        }
        commandArgs.push_back(arg);
    }

    const SIYI::TransportProtocol protocol =
        (transportName == "tcp") ? SIYI::TransportProtocol::TCP : SIYI::TransportProtocol::UDP;

    std::string connectError;
    auto manager = SIYI::CameraManager::Connect(ipAddress, port, protocol, timeoutMs, &connectError);
    if (!manager.has_value())
    {
        std::cerr << "camera connect failed: " << connectError << "\n";
        return 1;
    }

    auto& camera = manager->Camera();

    std::optional<std::vector<uint8_t>> frame;
    try
    {
        frame = BuildCommand(camera, commandArgs, needAck);
    }
    catch (const std::exception& ex)
    {
        std::cerr << "command build failed: " << ex.what() << "\n";
        return 1;
    }

    if (!frame.has_value())
    {
        PrintUsage(argv[0]);
        return 1;
    }

    if (frame->size() < 8U)
    {
        std::cerr << "built frame is unexpectedly short\n";
        return 1;
    }

    if (!commandArgs.empty() && commandArgs[0] == "tcp-heartbeat" && transportName != "tcp")
    {
        std::cerr << "tcp-heartbeat is only available when --transport tcp is selected\n";
        return 1;
    }

    const bool waitForAck = needAck && CommandExpectedToAck(commandArgs);
    if (needAck && !waitForAck)
    {
        std::cout << "Note: command is configured as no-ACK; skipping ACK wait.\n";
    }

    std::cout << "Sending frame: " << SIYI::BytesToHex(frame.value()) << "\n";
    std::cout << "Sequence: " << (camera.PeekNextSeq() - 1) << "\n";

    SIYI::ExecuteOptions execOptions;
    execOptions.waitForAck = waitForAck;
    execOptions.timeoutMs = timeoutMs;
    execOptions.repeatCount = repeatCount;
    execOptions.periodMs = periodMs;
    execOptions.bindPort = bindPort;

    const SIYI::ExecuteResult execResult = manager->Execute(ipAddress, port, protocol, frame.value(), execOptions);

    if (execResult.status != SIYI::ExecuteStatus::OK)
    {
        std::cerr << "command execution failed: " << execResult.message << "\n";
        return 1;
    }

    if (!execResult.lastResponseFrame.empty())
    {
        std::cout << "Received frame: " << SIYI::BytesToHex(execResult.lastResponseFrame) << "\n";
    }

    if (execResult.decodedPacket.has_value())
    {
        const SIYI::SIYIPacket& decoded = execResult.decodedPacket.value();
        std::cout << "Decoded response:\n";
        std::cout << "  ctrl: 0x" << std::hex << static_cast<unsigned int>(decoded.ctrl) << "\n";
        std::cout << "  seq: " << std::dec << decoded.seq << "\n";
        std::cout << "  cmd_id: 0x" << std::hex << static_cast<unsigned int>(decoded.cmdId) << "\n";
        std::cout << "  data_len: " << std::dec << decoded.dataLen << "\n";
        std::cout << "  crc16: 0x" << std::hex << decoded.crc16 << "\n";
        if (!decoded.data.empty())
        {
            std::cout << "  data: " << SIYI::BytesToHex(decoded.data) << "\n";
        }

        if (decoded.cmdId == static_cast<uint8_t>(SIYI::CommandId::SET_GIMBAL_ANGLE))
        {
            std::cout << "  ack_name: SetGimbalAngleAck\n";
        }
    }

    if (execResult.telemetry.has_value())
    {
        const SIYI::TM::TelemetryMessage& tmMessage = execResult.telemetry.value();
        if (const auto* att = std::get_if<SIYI::TM::GimbalAttitude>(&tmMessage); att != nullptr)
        {
            std::cout << "Decoded typed telemetry (Gimbal Attitude):\n";
            std::cout << "  yaw: " << att->yaw << " (" << att->YawDeg() << " deg)\n";
            std::cout << "  pitch: " << att->pitch << " (" << att->PitchDeg() << " deg)\n";
            std::cout << "  roll: " << att->roll << " (" << att->RollDeg() << " deg)\n";
            std::cout << "  yaw_velocity: " << att->yawVelocity << " (" << att->YawVelocityDegPerSec() << " deg/s)\n";
            std::cout << "  pitch_velocity: " << att->pitchVelocity << " (" << att->PitchVelocityDegPerSec() << " deg/s)\n";
            std::cout << "  roll_velocity: " << att->rollVelocity << " (" << att->RollVelocityDegPerSec() << " deg/s)\n";
        }
        else if (const auto* info = std::get_if<SIYI::TM::GimbalConfiguration>(&tmMessage); info != nullptr)
        {
            std::cout << "Decoded typed telemetry (Gimbal Configuration):\n";
            std::cout << "  reserved0: " << static_cast<unsigned int>(info->reserved0) << "\n";
            std::cout << "  hdr_status: " << static_cast<unsigned int>(info->hdrStatus) << "\n";
            std::cout << "  reserved1: " << static_cast<unsigned int>(info->reserved1) << "\n";
            std::cout << "  record_status: " << static_cast<unsigned int>(info->recordStatus) << "\n";
            std::cout << "  gimbal_motion_mode: " << static_cast<unsigned int>(info->gimbalMotionMode) << "\n";
            std::cout << "  gimbal_mounting_method: " << static_cast<unsigned int>(info->gimbalMountingMethod) << "\n";
            std::cout << "  video_hdmi_or_cvbs: " << static_cast<unsigned int>(info->video_hdmi_or_cvbs) << "\n";
        }
        else if (const auto* hardwareId = std::get_if<SIYI::TM::GimbalHardwareId>(&tmMessage); hardwareId != nullptr)
        {
            std::cout << "Decoded typed telemetry (GimbalHardwareId):\n";
            std::cout << "  gimbal_model: " << static_cast<unsigned int>(hardwareId->gimbalModel) << "\n";
        }
        else if (const auto* fw = std::get_if<SIYI::TM::FirmwareVersion>(&tmMessage); fw != nullptr)
        {
            std::cout << "Decoded typed telemetry (FirmwareVersion):\n";
            std::cout << "  camera_firmware_version: " << fw->cameraFirmwareVersion << "\n";
            std::cout << "  gimbal_firmware_version: " << fw->gimbalFirmwareVersion << "\n";
            std::cout << "  zoom_firmware_version: " << fw->zoomFirmwareVersion << "\n";
        }
        else if (const auto* ack = std::get_if<SIYI::TM::SetGimbalAngleAck>(&tmMessage); ack != nullptr)
        {
            std::cout << "Decoded typed telemetry (SetGimbalAngleAck):\n";
            std::cout << "  currentYawAngle: " << ack->currentYawAngle << " (" << ack->YawDeg() << " deg)\n";
            std::cout << "  currentPitchAngle: " << ack->currentPitchAngle << " (" << ack->PitchDeg() << " deg)\n";
            std::cout << "  currentRollAngle: " << ack->currentRollAngle << " (" << ack->RollDeg() << " deg)\n";
        }
        else if (const auto* info = std::get_if<SIYI::TM::FuncFeedbackInfo>(&tmMessage); info != nullptr)
        {
            std::cout << "Decoded typed telemetry (FuncFeedbackInfo):\n";
            std::cout << "  infoType: " << static_cast<unsigned int>(info->infoType) << "\n";
        }
        else if (const auto* ack = std::get_if<SIYI::TM::CommandStatusAck>(&tmMessage); ack != nullptr)
        {
            std::cout << "Decoded typed telemetry (CommandStatusAck):\n";
            std::cout << "  cmd_id: 0x" << std::hex << static_cast<unsigned int>(ack->cmdId) << std::dec << "\n";
            std::cout << "  command_name: " << ack->CommandName() << "\n";
            std::cout << "  status: " << static_cast<unsigned int>(ack->status)
                      << (ack->IsSuccess() ? " (success)" : " (failure)") << "\n";
        }
        else if (const auto* unknown = std::get_if<SIYI::TM::UnknownTelemetry>(&tmMessage); unknown != nullptr)
        {
            std::cout << "Decoded typed telemetry (UnknownTelemetry):\n";
            std::cout << "  cmd_id: 0x" << std::hex << static_cast<unsigned int>(unknown->cmdId) << std::dec << "\n";
            std::cout << "  data: " << SIYI::BytesToHex(unknown->data) << "\n";
        }
    }

    if (!waitForAck)
    {
        std::cout << "Command sent without ACK request.\n";
    }

    return 0;
}
