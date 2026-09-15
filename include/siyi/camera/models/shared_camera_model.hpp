#ifndef SHARED_CAMERA_BASE_HPP
#define SHARED_CAMERA_BASE_HPP

#include "../core/camera_command_interface.hpp"
#include "../core/telemetry_decoder_interface.hpp"
#include "../../helper/ilog_manager.hpp"

#include <array>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>
#include <chrono>

namespace SIYI
{
    struct CameraResolution
    {
        uint16_t width {0};
        uint16_t height {0};
    };

    struct VideoRecordingResolution
    {
        CameraResolution resolution {};
        uint8_t fps {0};
    };

    struct CameraTechnicalSpecs
    {
        float angularVibrationRangeDeg {0.0F};
        std::string sensorDescription;
        float effectiveResolutionMegaPixels {0.0F};
        std::vector<VideoRecordingResolution> supportedVideoRecordingResolutions;
        std::vector<CameraResolution> supportedStillPhotoResolutions;
        std::string imageFormat;
        std::string videoFileFormat;
    };

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

    uint16_t Crc16Ccitt(const std::vector<uint8_t>& data);
    std::optional<std::vector<uint8_t>> HexToBytes(const std::string& hex, std::string* error = nullptr);
    std::string BytesToHex(const std::vector<uint8_t>& data);

    class SharedCameraModel : public ICommandCamera, public ITelemetryDecoder {
        public:
            virtual ~SharedCameraModel() = default;

            explicit SharedCameraModel(int16_t pitchMin, int16_t pitchMax,
                                       int16_t yawMin, int16_t yawMax,
                                       int16_t rollMin, int16_t rollMax,
                                       std::shared_ptr<TelecommandSession> session = std::make_shared<TelecommandSession>(),
                                       const CameraTechnicalSpecs& technicalSpecs = CameraTechnicalSpecs());

            explicit SharedCameraModel(int16_t pitchMin, int16_t pitchMax,
                                       int16_t yawMin, int16_t yawMax,
                                       int16_t rollMin, int16_t rollMax,
                                       int16_t zoomMin, int16_t zoomMax,
                                       std::shared_ptr<TelecommandSession> session = std::make_shared<TelecommandSession>(),
                                       const CameraTechnicalSpecs& technicalSpecs = CameraTechnicalSpecs());

            std::vector<uint8_t> StartRotation(int8_t yawSpeed, int8_t pitchSpeed) const;
            
            std::vector<uint8_t> StopRotation() const;
            
            std::vector<uint8_t> SetGimbalAngle(int16_t yaw, int16_t pitch) const;
            
            std::vector<uint8_t> SetGimbalAngle(float yaw, float pitch) const;

            std::vector<uint8_t> ControlPhotoRecord(PhotoRecordFunction funcType) const;

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

            std::vector<uint8_t> SetUtcTime(uint64_t unixTimeUs) const;

            std::vector<uint8_t> SetUtcTime(std::chrono::microseconds unixTime) const;

            std::vector<uint8_t> SoftRestart(uint8_t camera_reboot = 0, uint8_t gimbal_reset = 0) const;

            std::vector<uint8_t> SoftRestart(bool rebootCamera, bool resetGimbal) const;

            std::vector<uint8_t> AcquireCameraCodecSpecs(TM::StreamType streamType) const;

            std::vector<uint8_t> AcquireGimbalWorkingMode() const;

            std::vector<uint8_t> FormatSDCard() const;

            std::vector<uint8_t> SendCameraCodecSpecs(
                TM::StreamType streamType,
                TM::VideoEncType encType,
                uint16_t resolutionWidth,
                uint16_t resolutionHeight,
                uint16_t bitrateKbps) const;

            std::vector<uint8_t> BuildCustomCommand(uint8_t cmdId, const std::vector<uint8_t>& payload, bool needAck = true) const override;


            // accessors
            int16_t GetPitchMin() const noexcept { return pitch_min_; }
            int16_t GetPitchMax() const noexcept { return pitch_max_; }
            int16_t GetYawMin() const noexcept { return yaw_min_; }
            int16_t GetYawMax() const noexcept { return yaw_max_; }
            int16_t GetRollMin() const noexcept { return roll_min_; }
            int16_t GetRollMax() const noexcept { return roll_max_; }
            int16_t GetZoomMin() const noexcept { return zoom_min_; }
            int16_t GetZoomMax() const noexcept { return zoom_max_; }
            const CameraTechnicalSpecs& GetTechnicalSpecs() const noexcept { return technical_specs_; }
            uint16_t GetCurrentResolutionWidth() const noexcept { return current_resolution_width_; }
            uint16_t GetCurrentResolutionHeight() const noexcept { return current_resolution_height_; }
            bool HasCurrentResolution() const noexcept { return current_resolution_width_ > 0 && current_resolution_height_ > 0; }

            // transport
            bool DecodeFrame(
                const std::vector<uint8_t>& frame,
                SIYIPacket& outPacket,
                std::string* error = nullptr) const override;

            bool DecodeTelemetryPacket(
                const SIYIPacket& packet,
                TM::TelemetryMessage& outMessage,
                std::string* error = nullptr) const override;

            virtual bool DecodeTelemetryFrame(
                const std::vector<uint8_t>& frame,
                TM::TelemetryMessage& outMessage,
                std::string* error = nullptr) const;

        protected:
            std::vector<uint8_t> SetGimbalAngleRaw(int16_t yaw, int16_t pitch) const;

            void SetCameraName(const std::string& cameraName);

            bool IsPitchWithin(int16_t value) const noexcept { return value >= pitch_min_ && value <= pitch_max_; }
            bool IsPitchWithin(float value) const noexcept { return value >= static_cast<float>(pitch_min_) && value <= static_cast<float>(pitch_max_); }
            bool IsYawWithin(int16_t value) const noexcept { return value >= yaw_min_ && value <= yaw_max_; }
            bool IsYawWithin(float value) const noexcept { return value >= static_cast<float>(yaw_min_) && value <= static_cast<float>(yaw_max_); }
            std::vector<uint8_t> BuildPacket(
                uint8_t cmdId,
                const std::vector<uint8_t>& payload,
                ControlFlag flag) const;

            int16_t pitch_min_ {0};
            int16_t pitch_max_ {0};
            int16_t yaw_min_ {0};
            int16_t yaw_max_ {0};
            int16_t roll_min_ {0};
            int16_t roll_max_ {0};
            int16_t zoom_min_ {1};
            int16_t zoom_max_ {1};
            CameraTechnicalSpecs technical_specs_ {};
            mutable uint16_t current_resolution_width_ {0};
            mutable uint16_t current_resolution_height_ {0};
            mutable TM::StreamType pending_codec_stream_type_ {TM::StreamType::MAIN_STREAM};
            mutable uint16_t pending_resolution_width_ {0};
            mutable uint16_t pending_resolution_height_ {0};
            
            std::shared_ptr<helper::ILogManager> logger_;
            std::string cameraName_ = "undefined";
        };
}

#endif // SHARED_CAMERA_BASE_HPP
