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
                int8_t pitchSpeed,
                bool needAck = true) const override;

            std::vector<uint8_t> StopRotation(
                bool needAck = false) const override;

            std::vector<uint8_t> ControlPhotoRecord(
                PhotoRecordFunction funcType) const override;

            // Convenience wrappers for common photo/record operations.
            std::vector<uint8_t> TakePicture() const;

            std::vector<uint8_t> ToggleHDR() const;

            std::vector<uint8_t> StartStopRecording() const;

            std::vector<uint8_t> LockMotion() const;

            std::vector<uint8_t> FollowMotion() const;

            std::vector<uint8_t> FPVMotion() const;

            std::vector<uint8_t> SetManualZoom(
                const ManualZoomDirection direction,
                const bool needAck) const
            {
                const std::vector<uint8_t> payload = {static_cast<uint8_t>(direction)};
                return BuildPacket(
                    static_cast<uint8_t>(CommandId::ZOOM),
                    payload,
                    needAck ? ControlFlag::NEED_ACK : ControlFlag::NO_ACK);
            }

            std::vector<uint8_t> SetAbsoluteZoom(
                const int8_t zoomSpeed,
                const bool needAck) const
            {
                const std::vector<uint8_t> payload = {static_cast<uint8_t>(zoomSpeed)};
                return BuildPacket(
                    static_cast<uint8_t>(CommandId::ZOOM),
                    payload,
                    needAck ? ControlFlag::NEED_ACK : ControlFlag::NO_ACK);
            }
            
        // ACQUIRE_FW_VER = 0x01,
        // ACQUIRE_HW_ID = 0x02,
        // AUTO_FOCUS = 0x04,
        // CENTER = 0x08,
        // ACQUIRE_GIMBAL_CONFIGURATION = 0x0a,
        // FUNC_FEEDBACK_INFO = 0x0b,
        // ABSOLUTE_ZOOM = 0x0f,

            std::vector<uint8_t> AcquireFirmwareVersion() const
            {
                return BuildPacket(
                    static_cast<uint8_t>(CommandId::ACQUIRE_FW_VER),
                    {},
                    ControlFlag::NEED_ACK);
            }

            std::vector<uint8_t> AcquireHardwareId() const
            {
                return BuildPacket(
                    static_cast<uint8_t>(CommandId::ACQUIRE_HW_ID),
                    {},
                    ControlFlag::NEED_ACK);
            }


            // only availale for the Optical zoom gimbal cameras ZT30, ZR30 and ZR10 -> to be moved in i zoom camera interface when it's created
            std::vector<uint8_t> AutoFocus(uint16_t x_coord, uint16_t y_coord) const
            {
                std::vector<uint8_t> payload = {
                    0x01, // AF start
                    static_cast<uint8_t>((x_coord >> 8) & 0xFF),
                    static_cast<uint8_t>(x_coord & 0xFF),
                    static_cast<uint8_t>((y_coord >> 8) & 0xFF),
                    static_cast<uint8_t>(y_coord & 0xFF)};
                return BuildPacket(
                    static_cast<uint8_t>(CommandId::AUTO_FOCUS),
                    payload,
                    ControlFlag::NEED_ACK);
            }

            std::vector<uint8_t> Center() const
            {
                return BuildPacket(
                    static_cast<uint8_t>(CommandId::CENTER),
                    {0x1},
                    ControlFlag::NEED_ACK);
            }

            std::vector<uint8_t> AcquireGimbalConfiguration() const
            {
                return BuildPacket(
                    static_cast<uint8_t>(CommandId::ACQUIRE_GIMBAL_CONFIGURATION),
                    {},
                    ControlFlag::NEED_ACK);
            }

            // # for tm, must devide the value by 10 to get degree value
            std::vector<uint8_t> AcquireGimbalAttitude() const
            {
                return BuildPacket(
                    static_cast<uint8_t>(CommandId::ACQUIRE_GIMBAL_ATT),
                    {},
                    ControlFlag::NEED_ACK);
            }

            std::vector<uint8_t> AcquireFunctionFeedbackInfo() const
            {
                return BuildPacket(
                    static_cast<uint8_t>(CommandId::FUNC_FEEDBACK_INFO),
                    {},
                    ControlFlag::NEED_ACK);
            }

            std::vector<uint8_t> SetAbsoluteZoom(uint8_t int_zoomValue, uint8_t frac_zoomValue) const
            {
                return BuildPacket(
                    static_cast<uint8_t>(CommandId::ABSOLUTE_ZOOM),
                    {int_zoomValue, frac_zoomValue},
                    ControlFlag::NEED_ACK);
            }

            std::vector<uint8_t> SetAbsoluteZoom(float zoomSpeed) const
            {
                uint8_t intPart = static_cast<uint8_t>(zoomSpeed);
                uint8_t fracPart = static_cast<uint8_t>((zoomSpeed - intPart) * 10);
                return SetAbsoluteZoom(intPart, fracPart);
            }

            std::vector<uint8_t> SetUtcTime(uint64_t unixTimeUs) const
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

            std::vector<uint8_t> SetUtcTime(std::chrono::microseconds unixTime) const
            {
                return SetUtcTime(static_cast<uint64_t>(unixTime.count()));
            }

            std::vector<uint8_t> SoftRestart(uint8_t camera_reboot = 0, uint8_t gimbal_reset = 0) const
            {
                return BuildPacket(
                    static_cast<uint8_t>(CommandId::SOFT_RESTART),
                    {camera_reboot, gimbal_reset},
                    ControlFlag::NEED_ACK);
            }

            std::vector<uint8_t> SoftRestart(bool rebootCamera, bool resetGimbal) const
            {
                return BuildPacket(
                    static_cast<uint8_t>(CommandId::SOFT_RESTART),
                    {static_cast<uint8_t>(rebootCamera), static_cast<uint8_t>(resetGimbal)},
                    ControlFlag::NEED_ACK);
            }

            std::vector<uint8_t> BuildCustomCommand(
                uint8_t cmdId,
                const std::vector<uint8_t>& payload,
                bool needAck = false) const override;


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