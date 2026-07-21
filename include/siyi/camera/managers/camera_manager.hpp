#ifndef SIYI_CAMERA_MANAGER_HPP
#define SIYI_CAMERA_MANAGER_HPP

#include "../core/ack_policy.hpp"
#include "../models/shared_camera_model.hpp"
#include "../../transport/itransport.hpp"

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace SIYI
{
    enum class TransportProtocol
    {
        UDP,
        TCP
    };

    enum class ExecuteStatus
    {
        OK,
        INVALID_REQUEST_FRAME,
        TRANSPORT_OPEN_FAILED,
        SEND_FAILED,
        RECEIVE_FAILED,
        DECODE_FAILED,
        UNEXPECTED_ACK,
        NO_ACK_RECEIVED,
        UNSUPPORTED_CAMERA_MODEL
    };

    struct ExecuteOptions
    {
        bool waitForAck {true};
        int timeoutMs {1000};
        int repeatCount {1};
        int periodMs {100};
        uint16_t bindPort {0};
    };

    struct ExecuteResult
    {
        ExecuteStatus status {ExecuteStatus::OK};
        std::string message;
        std::vector<uint8_t> requestFrame;
        std::vector<uint8_t> lastResponseFrame;
        std::optional<SIYIPacket> decodedPacket;
        std::optional<TM::TelemetryMessage> telemetry;
        bool receivedAck {false};
    };

    class CameraManager
    {
    public:
        explicit CameraManager(std::shared_ptr<SharedCameraModel> camera);

        SharedCameraModel& Camera() const;

        /// Connects to a camera at the given address, sends AcquireHardwareId,
        /// detects the gimbal model, and returns a CameraManager owning the
        /// correctly-typed camera model. Returns nullopt on failure with a
        /// description in *error (if provided).
        static std::optional<CameraManager> Connect(
            const std::string& ipAddress,
            uint16_t port,
            TransportProtocol protocol,
            int timeoutMs = 2000,
            std::string* error = nullptr);

        ExecuteResult Execute(
            const std::string& ipAddress,
            uint16_t port,
            TransportProtocol protocol,
            const std::vector<uint8_t>& requestFrame,
            const ExecuteOptions& options) const;

    private:
        static std::unique_ptr<ITransport> MakeTransport(TransportProtocol protocol);

        std::shared_ptr<SharedCameraModel> camera_;
        std::unique_ptr<ITransport> transport_;
    };
} // namespace SIYI

#endif // SIYI_CAMERA_MANAGER_HPP