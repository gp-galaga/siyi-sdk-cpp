#ifndef __SIYI_CAMERA_HPP__
#define __SIYI_CAMERA_HPP__

#include "icamera_tc.hpp"

#include <array>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>
#include <chrono>

namespace SIYI
{
    struct SIYIPacket
    {
        static constexpr uint8_t STX_LOW = 0x55;
        static constexpr uint8_t STX_HIGH = 0x66;

        std::array<uint8_t, 2> stx {STX_LOW, STX_HIGH};
        uint8_t ctrl {static_cast<uint8_t>(ControlFlag::NO_ACK)};
        uint16_t dataLen {0};
        uint16_t seq {0};
        uint8_t cmdId {0};
        std::vector<uint8_t> data;
        uint16_t crc16 {0};

        std::vector<uint8_t> Encode() const;
        static std::optional<SIYIPacket> Decode(
            const std::vector<uint8_t>& frame,
            std::string* error = nullptr);
    };

    struct AngleLimit
    {
        int16_t min;
        int16_t max;

        int16_t Clamp(int16_t value) const
        {
            if (value < min)
                return min;
            if (value > max)
                return max;
            return value;
        }
    };

    uint16_t Crc16Ccitt(const std::vector<uint8_t>& data);
    std::optional<std::vector<uint8_t>> HexToBytes(const std::string& hex, std::string* error = nullptr);
    std::string BytesToHex(const std::vector<uint8_t>& data);

    class SIYICameraBase : public ITCCamera {
        public:
            explicit SIYICameraBase(
                std::shared_ptr<TelecommandSession> session = std::make_shared<TelecommandSession>());

            std::vector<uint8_t> StartRotation(
                int8_t yawSpeed,
                int8_t pitchSpeed) const override;

            std::vector<uint8_t> SetGimbalAngle(
                int16_t yaw,
                int16_t pitch) const override;

            std::vector<uint8_t> StopRotation() const override;

            std::vector<uint8_t> ControlPhotoRecord(PhotoRecordFunction funcType) const override;

            // Convenience wrappers for common photo/record operations.
            std::vector<uint8_t> TakePicture() const;

            std::vector<uint8_t> ToggleHDR() const;

            std::vector<uint8_t> StartStopRecording() const;

            std::vector<uint8_t> LockMotion() const;

            std::vector<uint8_t> FollowMotion() const;

            std::vector<uint8_t> FPVMotion() const;

            std::vector<uint8_t> AcquireFirmwareVersion() const;

            std::vector<uint8_t> AcquireHardwareId() const;

            std::vector<uint8_t> Center() const;

            std::vector<uint8_t> AcquireGimbalConfiguration() const;

            std::vector<uint8_t> AcquireGimbalAttitude() const;

            std::vector<uint8_t> AcquireFunctionFeedbackInfo() const;

            std::vector<uint8_t> SetUtcTime(uint64_t unixTimeUs) const;

            std::vector<uint8_t> SetUtcTime(std::chrono::microseconds unixTime) const;

            std::vector<uint8_t> SoftRestart(uint8_t camera_reboot = 0, uint8_t gimbal_reset = 0) const;

            std::vector<uint8_t> SoftRestart(bool rebootCamera, bool resetGimbal) const;

            std::vector<uint8_t> BuildCustomCommand(uint8_t cmdId, const std::vector<uint8_t>& payload, bool needAck = true) const override;


            bool DecodeFrame(
                const std::vector<uint8_t>& frame,
                SIYIPacket& outPacket,
                std::string* error = nullptr) const override;

            bool DecodeTelemetryPacket(
                const SIYIPacket& packet,
                TM::TelemetryMessage& outMessage,
                std::string* error = nullptr) const override;

            bool DecodeTelemetryFrame(
                const std::vector<uint8_t>& frame,
                TM::TelemetryMessage& outMessage,
                std::string* error = nullptr) const override;

        protected:
            std::vector<uint8_t> BuildPacket(
                uint8_t cmdId,
                const std::vector<uint8_t>& payload,
                ControlFlag flag) const;

            AngleLimit pitchLimit_ {};
            AngleLimit yawLimit_ {};
            AngleLimit zoomLimit_ {};
            AngleLimit rollLimit_ {};
        };
}

#endif // __SIYI_CAMERA_HPP__