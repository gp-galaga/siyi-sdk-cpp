#include "../../include/siyi/camera/models/shared_camera_model.hpp"
#include "../../include/siyi/helper/log_manager.hpp"

#include <algorithm>
#include <array>
#include <cctype>
#include <cstdlib>
#include <iomanip>
#include <stdexcept>
#include <sstream>
#include <math.h>

namespace SIYI
{
    namespace CRC16
    {
        constexpr std::array<uint16_t, 256> kCrcTable = {
            0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50A5, 0x60C6, 0x70E7,
            0x8108, 0x9129, 0xA14A, 0xB16B, 0xC18C, 0xD1AD, 0xE1CE, 0xF1EF,
            0x1231, 0x0210, 0x3273, 0x2252, 0x52B5, 0x4294, 0x72F7, 0x62D6,
            0x9339, 0x8318, 0xB37B, 0xA35A, 0xD3BD, 0xC39C, 0xF3FF, 0xE3DE,
            0x2462, 0x3443, 0x0420, 0x1401, 0x64E6, 0x74C7, 0x44A4, 0x5485,
            0xA56A, 0xB54B, 0x8528, 0x9509, 0xE5EE, 0xF5CF, 0xC5AC, 0xD58D,
            0x3653, 0x2672, 0x1611, 0x0630, 0x76D7, 0x66F6, 0x5695, 0x46B4,
            0xB75B, 0xA77A, 0x9719, 0x8738, 0xF7DF, 0xE7FE, 0xD79D, 0xC7BC,
            0x48C4, 0x58E5, 0x6886, 0x78A7, 0x0840, 0x1861, 0x2802, 0x3823,
            0xC9CC, 0xD9ED, 0xE98E, 0xF9AF, 0x8948, 0x9969, 0xA90A, 0xB92B,
            0x5AF5, 0x4AD4, 0x7AB7, 0x6A96, 0x1A71, 0x0A50, 0x3A33, 0x2A12,
            0xDBFD, 0xCBDC, 0xFBBF, 0xEB9E, 0x9B79, 0x8B58, 0xBB3B, 0xAB1A,
            0x6CA6, 0x7C87, 0x4CE4, 0x5CC5, 0x2C22, 0x3C03, 0x0C60, 0x1C41,
            0xEDAE, 0xFD8F, 0xCDEC, 0xDDCD, 0xAD2A, 0xBD0B, 0x8D68, 0x9D49,
            0x7E97, 0x6EB6, 0x5ED5, 0x4EF4, 0x3E13, 0x2E32, 0x1E51, 0x0E70,
            0xFF9F, 0xEFBE, 0xDFDD, 0xCFFC, 0xBF1B, 0xAF3A, 0x9F59, 0x8F78,
            0x9188, 0x81A9, 0xB1CA, 0xA1EB, 0xD10C, 0xC12D, 0xF14E, 0xE16F,
            0x1080, 0x00A1, 0x30C2, 0x20E3, 0x5004, 0x4025, 0x7046, 0x6067,
            0x83B9, 0x9398, 0xA3FB, 0xB3DA, 0xC33D, 0xD31C, 0xE37F, 0xF35E,
            0x02B1, 0x1290, 0x22F3, 0x32D2, 0x4235, 0x5214, 0x6277, 0x7256,
            0xB5EA, 0xA5CB, 0x95A8, 0x8589, 0xF56E, 0xE54F, 0xD52C, 0xC50D,
            0x34E2, 0x24C3, 0x14A0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
            0xA7DB, 0xB7FA, 0x8799, 0x97B8, 0xE75F, 0xF77E, 0xC71D, 0xD73C,
            0x26D3, 0x36F2, 0x0691, 0x16B0, 0x6657, 0x7676, 0x4615, 0x5634,
            0xD94C, 0xC96D, 0xF90E, 0xE92F, 0x99C8, 0x89E9, 0xB98A, 0xA9AB,
            0x5844, 0x4865, 0x7806, 0x6827, 0x18C0, 0x08E1, 0x3882, 0x28A3,
            0xCB7D, 0xDB5C, 0xEB3F, 0xFB1E, 0x8BF9, 0x9BD8, 0xABBB, 0xBB9A,
            0x4A75, 0x5A54, 0x6A37, 0x7A16, 0x0AF1, 0x1AD0, 0x2AB3, 0x3A92,
            0xFD2E, 0xED0F, 0xDD6C, 0xCD4D, 0xBDAA, 0xAD8B, 0x9DE8, 0x8DC9,
            0x7C26, 0x6C07, 0x5C64, 0x4C45, 0x3CA2, 0x2C83, 0x1CE0, 0x0CC1,
            0xEF1F, 0xFF3E, 0xCF5D, 0xDF7C, 0xAF9B, 0xBFBA, 0x8FD9, 0x9FF8,
            0x6E17, 0x7E36, 0x4E55, 0x5E74, 0x2E93, 0x3EB2, 0x0ED1, 0x1EF0};

        uint16_t ReadU16Le(const std::vector<uint8_t> &data, size_t index)
        {
            return static_cast<uint16_t>(data[index]) |
                   static_cast<uint16_t>(data[index + 1] << 8);
        }

        void PushU16Le(std::vector<uint8_t> &data, uint16_t value)
        {
            data.push_back(static_cast<uint8_t>(value & 0xFF));
            data.push_back(static_cast<uint8_t>((value >> 8) & 0xFF));
        }

        int16_t ReadI16Le(const std::vector<uint8_t> &data, size_t index)
        {
            return static_cast<int16_t>(
                static_cast<uint16_t>(data[index]) |
                static_cast<uint16_t>(data[index + 1] << 8));
        }

        uint32_t ReadU32Le(const std::vector<uint8_t> &data, size_t index)
        {
            return static_cast<uint32_t>(data[index]) |
                   (static_cast<uint32_t>(data[index + 1]) << 8) |
                   (static_cast<uint32_t>(data[index + 2]) << 16) |
                   (static_cast<uint32_t>(data[index + 3]) << 24);
        }
    } // namespace CRC16

    SharedCameraModel::SharedCameraModel(int16_t pitchMin, int16_t pitchMax,
                                         int16_t yawMin, int16_t yawMax,
                                         int16_t rollMin, int16_t rollMax,
                                         std::shared_ptr<TelecommandSession> session)
        : ICommandCamera(std::move(session)),
                    logger_(helper::CreateConsoleLogger("SharedCameraModel")),
                    cameraName_("SharedCameraModel"),
          pitch_min_(pitchMin),
          pitch_max_(pitchMax),
          yaw_min_(yawMin),
          yaw_max_(yawMax),
          roll_min_(rollMin),
          roll_max_(rollMax) {
          };

    SharedCameraModel::SharedCameraModel(int16_t pitchMin, int16_t pitchMax,
                                         int16_t yawMin, int16_t yawMax,
                                         int16_t rollMin, int16_t rollMax,
                                         int16_t zoomMin, int16_t zoomMax,
                                         std::shared_ptr<TelecommandSession> session)
        : SharedCameraModel(pitchMin, pitchMax, yawMin, yawMax, rollMin, rollMax, std::move(session)) {
        zoom_min_ = zoomMin;
        zoom_max_ = zoomMax;
    }

    void SharedCameraModel::SetCameraName(const std::string &cameraName)
    {
        cameraName_ = cameraName;
        logger_ = helper::CreateConsoleLogger(cameraName_);
    }

    uint16_t Crc16Ccitt(const std::vector<uint8_t> &data)
    {
        uint16_t crc = 0x0000;
        for (const uint8_t byte : data)
        {
            const uint8_t tableIndex = static_cast<uint8_t>(((crc >> 8) & 0xFF) ^ byte);
            crc = static_cast<uint16_t>(((crc << 8) & 0xFF00) ^ CRC16::kCrcTable[tableIndex]);
        }
        return crc;
    }

    std::optional<std::vector<uint8_t>> HexToBytes(const std::string &hex, std::string *error)
    {
        std::string clean;
        clean.reserve(hex.size());
        for (const char c : hex)
        {
            if (!std::isspace(static_cast<unsigned char>(c)))
            {
                clean.push_back(c);
            }
        }

        if (clean.empty())
        {
            if (error != nullptr)
            {
                *error = "hex string is empty";
            }
            return std::nullopt;
        }

        if ((clean.size() % 2U) != 0U)
        {
            if (error != nullptr)
            {
                *error = "hex string must contain an even number of characters";
            }
            return std::nullopt;
        }

        std::vector<uint8_t> out;
        out.reserve(clean.size() / 2U);

        for (size_t i = 0; i < clean.size(); i += 2)
        {
            const char high = clean[i];
            const char low = clean[i + 1];
            if (!std::isxdigit(static_cast<unsigned char>(high)) ||
                !std::isxdigit(static_cast<unsigned char>(low)))
            {
                if (error != nullptr)
                {
                    *error = "hex string contains a non-hex character";
                }
                return std::nullopt;
            }

            const uint8_t value = static_cast<uint8_t>(std::stoi(clean.substr(i, 2), nullptr, 16));
            out.push_back(value);
        }

        return out;
    }

    std::string BytesToHex(const std::vector<uint8_t> &data)
    {
        std::ostringstream oss;
        oss << std::uppercase << std::hex << std::setfill('0');
        for (const uint8_t b : data)
        {
            oss << std::setw(2) << static_cast<unsigned int>(b);
        }
        return oss.str();
    }

    std::vector<uint8_t> SIYIPacket::Encode() const
    {
        std::vector<uint8_t> out;
        out.reserve(10U + data.size());

        out.push_back(stx[0]);
        out.push_back(stx[1]);
        out.push_back(ctrl);

        const uint16_t payloadLen = static_cast<uint16_t>(data.size());
        CRC16::PushU16Le(out, payloadLen);
        CRC16::PushU16Le(out, seq);
        out.push_back(cmdId);

        out.insert(out.end(), data.begin(), data.end());

        const uint16_t crc = Crc16Ccitt(out);
        CRC16::PushU16Le(out, crc);
        return out;
    }

    std::optional<SIYIPacket> SIYIPacket::Decode(const std::vector<uint8_t> &frame, std::string *error)
    {
        constexpr size_t kMinimumFrameSize = 10U;
        if (frame.size() < kMinimumFrameSize)
        {
            if (error != nullptr)
            {
                *error = "frame too short";
            }
            return std::nullopt;
        }

        if (frame[0] != STX_LOW || frame[1] != STX_HIGH)
        {
            if (error != nullptr)
            {
                *error = "invalid STX";
            }
            return std::nullopt;
        }

        const uint16_t dataLen = CRC16::ReadU16Le(frame, 3);
        const size_t expectedSize = 8U + static_cast<size_t>(dataLen) + 2U;
        if (frame.size() != expectedSize)
        {
            if (error != nullptr)
            {
                *error = "frame size does not match Data_len";
            }
            return std::nullopt;
        }

        const std::vector<uint8_t> withoutCrc(frame.begin(), frame.end() - 2);
        const uint16_t expectedCrc = Crc16Ccitt(withoutCrc);
        const uint16_t frameCrc = CRC16::ReadU16Le(frame, frame.size() - 2U);
        if (frameCrc != expectedCrc)
        {
            if (error != nullptr)
            {
                *error = "CRC mismatch";
            }
            return std::nullopt;
        }

        SIYIPacket packet;
        packet.stx = {frame[0], frame[1]};
        packet.ctrl = frame[2];
        packet.dataLen = dataLen;
        packet.seq = CRC16::ReadU16Le(frame, 5);
        packet.cmdId = frame[7];
        packet.data.assign(frame.begin() + 8, frame.begin() + 8 + dataLen);
        packet.crc16 = frameCrc;

        return packet;
    }

    std::vector<uint8_t> SharedCameraModel::BuildPacket(
        const uint8_t cmdId,
        const std::vector<uint8_t> &payload,
        const ControlFlag flag) const
    {
        SIYIPacket packet;
        packet.ctrl = static_cast<uint8_t>(flag);
        packet.seq = IncrementSeq();
        packet.cmdId = cmdId;
        packet.data = payload;
        packet.dataLen = static_cast<uint16_t>(payload.size());
        return packet.Encode();
    }

    std::vector<uint8_t> SharedCameraModel::SetGimbalAngleRaw(int16_t yaw, int16_t pitch) const
    {
        const std::vector<uint8_t> payload = {
            static_cast<uint8_t>(yaw & 0xFF),
            static_cast<uint8_t>((yaw >> 8) & 0xFF),
            static_cast<uint8_t>(pitch & 0xFF),
            static_cast<uint8_t>((pitch >> 8) & 0xFF)};
        return BuildPacket(
            static_cast<uint8_t>(CommandId::SET_GIMBAL_ANGLE),
            payload,
            ControlFlag::NEED_ACK);
    }

    std::vector<uint8_t> SharedCameraModel::SetGimbalAngle(int16_t yaw, int16_t pitch) const
    {
        if (!IsYawWithin(yaw) || !IsPitchWithin(pitch))
        {
            throw std::out_of_range(
                "yaw must be in [" + std::to_string(yaw_min_) + ", " + std::to_string(yaw_max_) +
                "] and pitch must be in [" + std::to_string(pitch_min_) + ", " + std::to_string(pitch_max_) + "]");
        }
        return SetGimbalAngleRaw(yaw * 10, pitch * 10);
    }

    std::vector<uint8_t> SharedCameraModel::SetGimbalAngle(float yaw, float pitch) const
    {
        if (!IsYawWithin(yaw) || !IsPitchWithin(pitch))
        {
            throw std::out_of_range(
                "yaw must be in [" + std::to_string(yaw_min_) + ", " + std::to_string(yaw_max_) +
                "] and pitch must be in [" + std::to_string(pitch_min_) + ", " + std::to_string(pitch_max_) + "]");
        }
        return SetGimbalAngleRaw(
            static_cast<int16_t>(std::lround(yaw * 10.0F)),
            static_cast<int16_t>(std::lround(pitch * 10.0F)));
    }

    std::vector<uint8_t> SharedCameraModel::StartRotation(const int8_t yawSpeed, const int8_t pitchSpeed) const
    {
        const std::vector<uint8_t> payload = {
            static_cast<uint8_t>(yawSpeed),
            static_cast<uint8_t>(pitchSpeed)};
        return BuildPacket(
            static_cast<uint8_t>(CommandId::ROTATION),
            payload,
            ControlFlag::NEED_ACK);
    }

    std::vector<uint8_t> SharedCameraModel::StopRotation() const
    {
        const std::vector<uint8_t> payload = {0x00, 0x00};
        return BuildPacket(
            static_cast<uint8_t>(CommandId::ROTATION),
            payload,
            ControlFlag::NEED_ACK);
    }

    std::vector<uint8_t> SharedCameraModel::AcquireFirmwareVersion() const
    {
        return BuildPacket(
            static_cast<uint8_t>(CommandId::ACQUIRE_FW_VER),
            {},
            ControlFlag::NEED_ACK);
    }

    std::vector<uint8_t> SharedCameraModel::AcquireHardwareId() const
    {
        return BuildPacket(
            static_cast<uint8_t>(CommandId::ACQUIRE_HW_ID),
            {},
            ControlFlag::NEED_ACK);
    }

    std::vector<uint8_t> SharedCameraModel::Center() const
    {
        return BuildPacket(
            static_cast<uint8_t>(CommandId::CENTER),
            {0x1},
            ControlFlag::NEED_ACK);
    }

    std::vector<uint8_t> SharedCameraModel::AcquireGimbalConfiguration() const
    {
        return BuildPacket(
            static_cast<uint8_t>(CommandId::ACQUIRE_GIMBAL_CONFIGURATION),
            {},
            ControlFlag::NEED_ACK);
    }

    std::vector<uint8_t> SharedCameraModel::AcquireGimbalAttitude() const
    {
        return BuildPacket(
            static_cast<uint8_t>(CommandId::ACQUIRE_GIMBAL_ATT),
            {},
            ControlFlag::NEED_ACK);
    }

    std::vector<uint8_t> SharedCameraModel::AcquireFunctionFeedbackInfo() const
    {
        return BuildPacket(
            static_cast<uint8_t>(CommandId::FUNC_FEEDBACK_INFO),
            {},
            ControlFlag::NEED_ACK);
    }

    std::vector<uint8_t> SharedCameraModel::SetUtcTime(uint64_t unixTimeUs) const
    {
        std::vector<uint8_t> payload(8);
        payload[0] = static_cast<uint8_t>((unixTimeUs >> 0) & 0xFF);

        payload[1] = static_cast<uint8_t>((unixTimeUs >> 8) & 0xFF);
        payload[2] = static_cast<uint8_t>((unixTimeUs >> 16) & 0xFF);
        payload[3] = static_cast<uint8_t>((unixTimeUs >> 24) & 0xFF);
        payload[4] = static_cast<uint8_t>((unixTimeUs >> 32) & 0xFF);
        payload[5] = static_cast<uint8_t>((unixTimeUs >> 40) & 0xFF);
        payload[6] = static_cast<uint8_t>((unixTimeUs >> 48) & 0xFF);
        payload[7] = static_cast<uint8_t>((unixTimeUs >> 56) & 0xFF);
        return BuildPacket(
            static_cast<uint8_t>(CommandId::SET_UTC_TIME),
            payload,
            ControlFlag::NEED_ACK);
    }

    std::vector<uint8_t> SharedCameraModel::SetUtcTime(std::chrono::microseconds unixTime) const
    {
        return SetUtcTime(static_cast<uint64_t>(unixTime.count()));
    }

    std::vector<uint8_t> SharedCameraModel::SoftRestart(uint8_t camera_reboot, uint8_t gimbal_reset) const
    {
        return BuildPacket(
            static_cast<uint8_t>(CommandId::SOFT_RESTART),
            {camera_reboot, gimbal_reset},
            ControlFlag::NEED_ACK);
    }

    std::vector<uint8_t> SharedCameraModel::SoftRestart(bool rebootCamera, bool resetGimbal) const
    {
        return BuildPacket(
            static_cast<uint8_t>(CommandId::SOFT_RESTART),
            {static_cast<uint8_t>(rebootCamera), static_cast<uint8_t>(resetGimbal)},
            ControlFlag::NEED_ACK);
    }

    std::vector<uint8_t> SharedCameraModel::SetManualZoom(ManualZoomDirection) const
    {
        throw std::logic_error(cameraName_ + " does not support manual zoom commands");
    }

    std::vector<uint8_t> SharedCameraModel::AutoFocus(uint16_t, uint16_t) const
    {
        throw std::logic_error(cameraName_ + " does not support auto focus commands");
    }

    std::vector<uint8_t> SharedCameraModel::SetManualFocus(ManualFocusDirection) const
    {
        throw std::logic_error(cameraName_ + " does not support manual focus commands");
    }

    std::vector<uint8_t> SharedCameraModel::SetAbsoluteZoom(float) const
    {
        throw std::logic_error(cameraName_ + " does not support absolute zoom commands");
    }

    std::vector<uint8_t> SharedCameraModel::SetAbsoluteZoom(int) const
    {
        throw std::logic_error(cameraName_ + " does not support absolute zoom commands");
    }

    std::vector<uint8_t> SharedCameraModel::ControlPhotoRecord(
        const PhotoRecordFunction funcType) const
    {
        const std::vector<uint8_t> payload = {static_cast<uint8_t>(funcType)};
        return BuildPacket(
            static_cast<uint8_t>(CommandId::PHOTO_RECORD),
            payload,
            ControlFlag::NO_ACK);
    }

    std::vector<uint8_t> SharedCameraModel::TakePicture() const
    {
        return ControlPhotoRecord(PhotoRecordFunction::TAKE_PICTURE);
    }

    // not supported yet
    std::vector<uint8_t> SharedCameraModel::ToggleHDR() const
    {
        return ControlPhotoRecord(PhotoRecordFunction::TOGGLE_HDR);
    }

    std::vector<uint8_t> SharedCameraModel::StartStopRecording() const
    {
        return ControlPhotoRecord(PhotoRecordFunction::START_STOP_RECORDING);
    }

    std::vector<uint8_t> SharedCameraModel::LockMotion() const
    {
        return ControlPhotoRecord(PhotoRecordFunction::MOTION_LOCK_MODE);
    }

    std::vector<uint8_t> SharedCameraModel::FollowMotion() const
    {
        return ControlPhotoRecord(PhotoRecordFunction::MOTION_FOLLOW_MODE);
    }

    std::vector<uint8_t> SharedCameraModel::FPVMotion() const
    {
        return ControlPhotoRecord(PhotoRecordFunction::MOTION_FPV_MODE);
    }

    std::vector<uint8_t> SharedCameraModel::BuildCustomCommand(
        const uint8_t cmdId,
        const std::vector<uint8_t> &payload,
        const bool needAck) const
    {
        return BuildPacket(
            cmdId,
            payload,
            needAck ? ControlFlag::NEED_ACK : ControlFlag::NO_ACK);
    }

    bool SharedCameraModel::DecodeFrame(
        const std::vector<uint8_t> &frame,
        SIYIPacket &outPacket,
        std::string *error) const
    {
        const auto decoded = SIYIPacket::Decode(frame, error);
        if (!decoded.has_value())
        {
            return false;
        }
        outPacket = decoded.value();
        return true;
    }

    bool SharedCameraModel::DecodeTelemetryPacket(
        const SIYIPacket &packet,
        TM::TelemetryMessage &outMessage,
        std::string *error) const
    {
        if (packet.cmdId == static_cast<uint8_t>(CommandId::ACQUIRE_HW_ID))
        {
            if (packet.data.empty())
            {
                if (error != nullptr)
                {
                    *error = "ACQUIRE_HW_ID payload must contain at least 1 byte";
                }
                return false;
            }

            TM::GimbalHardwareId hardwareId;
            hardwareId.gimbalModel = packet.data[0];

            if (packet.data.size() >= 2 &&
                std::isxdigit(static_cast<unsigned char>(packet.data[0])) &&
                std::isxdigit(static_cast<unsigned char>(packet.data[1])))
            {
                const std::string modelPrefix {
                    static_cast<char>(packet.data[0]),
                    static_cast<char>(packet.data[1])};
                hardwareId.gimbalModel = static_cast<uint8_t>(std::strtoul(modelPrefix.c_str(), nullptr, 16));
            }

            outMessage = hardwareId;
            return true;
        }

        if (packet.cmdId == static_cast<uint8_t>(CommandId::ACQUIRE_GIMBAL_CONFIGURATION))
        {
            constexpr size_t kExpectedLen = 7;
            if (packet.data.size() != kExpectedLen)
            {
                if (error != nullptr)
                {
                    *error = "ACQUIRE_GIMBAL_INFO payload must be exactly 7 bytes";
                }
                return false;
            }

            TM::GimbalConfiguration config;
            config.reserved0 = packet.data[0];
            config.hdrStatus = packet.data[1];
            config.reserved1 = packet.data[2];
            config.recordStatus = packet.data[3];
            config.gimbalMotionMode = packet.data[4];
            config.gimbalMountingMethod = packet.data[5];
            config.video_hdmi_or_cvbs = packet.data[6];

            outMessage = config;
            return true;
        }

        if (packet.cmdId == static_cast<uint8_t>(CommandId::ACQUIRE_GIMBAL_ATT))
        {
            constexpr size_t kExpectedLen = 12;
            if (packet.data.size() != kExpectedLen)
            {
                if (error != nullptr)
                {
                    *error = "ACQUIRE_GIMBAL_ATT payload must be exactly 12 bytes";
                }
                return false;
            }

            TM::GimbalAttitude attitude;
            attitude.yaw = CRC16::ReadI16Le(packet.data, 0);
            attitude.pitch = CRC16::ReadI16Le(packet.data, 2);
            attitude.roll = CRC16::ReadI16Le(packet.data, 4);
            attitude.yawVelocity = CRC16::ReadI16Le(packet.data, 6);
            attitude.pitchVelocity = CRC16::ReadI16Le(packet.data, 8);
            attitude.rollVelocity = CRC16::ReadI16Le(packet.data, 10);

            outMessage = attitude;
            return true;
        }

        if (packet.cmdId == static_cast<uint8_t>(CommandId::ACQUIRE_FW_VER))
        {
            constexpr size_t kExpectedLen = 12;
            if (packet.data.size() != kExpectedLen)
            {
                if (error != nullptr)
                {
                    *error = "ACQUIRE_FW_VER payload must be exactly 12 bytes";
                }
                return false;
            }

            TM::FirmwareVersion fwVersion;
            fwVersion.cameraFirmwareVersion = CRC16::ReadU32Le(packet.data, 0);
            fwVersion.gimbalFirmwareVersion = CRC16::ReadU32Le(packet.data, 4);
            fwVersion.zoomFirmwareVersion = CRC16::ReadU32Le(packet.data, 8);

            outMessage = fwVersion;
            return true;
        }

        if (packet.cmdId == static_cast<uint8_t>(CommandId::SET_GIMBAL_ANGLE))
        {
            constexpr size_t kExpectedLen = 6;
            if (packet.data.size() != kExpectedLen)
            {
                if (error != nullptr)
                {
                    *error = "SET_GIMBAL_ANGLE payload must be exactly 6 bytes";
                }
                return false;
            }

            TM::SetGimbalAngleAck ack;
            ack.currentYawAngle = CRC16::ReadI16Le(packet.data, 0);
            ack.currentPitchAngle = CRC16::ReadI16Le(packet.data, 2);
            ack.currentRollAngle = CRC16::ReadI16Le(packet.data, 4);

            outMessage = ack;
            return true;
        }

        if (packet.cmdId == static_cast<uint8_t>(CommandId::FUNC_FEEDBACK_INFO))
        {
            constexpr size_t kExpectedLen = 1;
            if (packet.data.size() != kExpectedLen)
            {
                if (error != nullptr)
                {
                    *error = "FUNC_FEEDBACK_INFO payload must be exactly 1 byte";
                }
                return false;
            }

            TM::FuncFeedbackInfo info;
            info.infoType = packet.data[0];

            outMessage = info;
            return true;
        }

        if (packet.data.size() == 1)
        {
            TM::CommandStatusAck statusAck;
            statusAck.cmdId = packet.cmdId;
            statusAck.status = packet.data[0];
            outMessage = statusAck;
            return true;
        }

        TM::UnknownTelemetry unknown;
        unknown.cmdId = packet.cmdId;
        unknown.data = packet.data;
        outMessage = std::move(unknown);
        if (error != nullptr)
        {
            *error = "unsupported telemetry cmd_id for typed decoding";
        }
        return true;
    }

    bool SharedCameraModel::DecodeTelemetryFrame(
        const std::vector<uint8_t> &frame,
        TM::TelemetryMessage &outMessage,
        std::string *error) const
    {
        SIYIPacket packet;
        if (!DecodeFrame(frame, packet, error))
        {
            return false;
        }

        return DecodeTelemetryPacket(packet, outMessage, error);
    }
} // namespace SIYI
