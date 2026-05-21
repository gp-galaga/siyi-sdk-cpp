#include "../../../include/siyi/camera/managers/camera_manager.hpp"
#include "../../../include/siyi/camera/factories/camera_model_factory.hpp"
#include "../../../include/siyi/transport/tcp_transport.hpp"
#include "../../../include/siyi/transport/udp_transport.hpp"

#include <chrono>
#include <thread>

namespace SIYI
{
    CameraManager::CameraManager(std::shared_ptr<SharedCameraModel> camera) : camera_(std::move(camera)) {}

    SharedCameraModel& CameraManager::Camera() const
    {
        return *camera_;
    }

    std::optional<CameraManager> CameraManager::Connect(
        const std::string& ipAddress,
        const uint16_t port,
        const TransportProtocol protocol,
        const int timeoutMs,
        std::string* error)
    {
        // Use a base SharedCameraModel as a probe — limits are irrelevant for AcquireHardwareId.
        auto probeCamera = std::make_shared<SharedCameraModel>(0, 0, 0, 0, 0, 0);
        CameraManager probeManager(probeCamera);

        ExecuteOptions opts;
        opts.waitForAck = true;
        opts.timeoutMs = timeoutMs;
        opts.repeatCount = 3;

        const ExecuteResult probeResult = probeManager.Execute(
            ipAddress, port, protocol, probeCamera->AcquireHardwareId(), opts);

        if (probeResult.status != ExecuteStatus::OK)
        {
            if (error != nullptr)
            {
                *error = "hardware ID query failed: " + probeResult.message;
            }
            return std::nullopt;
        }

        if (!probeResult.telemetry.has_value())
        {
            if (error != nullptr)
            {
                *error = "hardware ID response carried no telemetry";
                if (probeResult.decodedPacket.has_value())
                {
                    *error += ", cmd_id=0x" + BytesToHex({probeResult.decodedPacket->cmdId}) +
                              ", payload=" + BytesToHex(probeResult.decodedPacket->data);
                }
            }
            return std::nullopt;
        }

        const auto* hwId = std::get_if<TM::GimbalHardwareId>(&probeResult.telemetry.value());
        if (hwId == nullptr)
        {
            if (error != nullptr)
            {
                *error = "hardware ID response had unexpected telemetry type";
            }
            return std::nullopt;
        }

        const auto gimbalModel = static_cast<TM::GimbalModel>(hwId->gimbalModel);
        switch (gimbalModel)
        {
            case TM::GimbalModel::ZR30:
                break;
            default:
                if (error != nullptr)
                {
                    *error = "unsupported gimbal model: " + std::to_string(hwId->gimbalModel);
                    if (probeResult.decodedPacket.has_value())
                    {
                        *error += ", payload=" + BytesToHex(probeResult.decodedPacket->data);
                    }
                }
                return std::nullopt;
        }

            return CameraManager(CreateCameraModel(gimbalModel));
    }

    std::unique_ptr<ITransport> CameraManager::MakeTransport(const TransportProtocol protocol)
    {
        if (protocol == TransportProtocol::TCP)
        {
            return std::make_unique<TcpTransport>();
        }
        return std::make_unique<UdpTransport>();
    }

    ExecuteResult CameraManager::Execute(
        const std::string& ipAddress,
        const uint16_t port,
        const TransportProtocol protocol,
        const std::vector<uint8_t>& requestFrame,
        const ExecuteOptions& options) const
    {
        ExecuteResult result;
        result.requestFrame = requestFrame;

        if (requestFrame.size() < 8U)
        {
            result.status = ExecuteStatus::INVALID_REQUEST_FRAME;
            result.message = "request frame is unexpectedly short";
            return result;
        }

        const uint8_t requestCmdId = requestFrame[7];
        std::string error;

        std::unique_ptr<ITransport> transport = MakeTransport(protocol);
        if (!transport->Open(ipAddress, port, options.bindPort, &error))
        {
            result.status = ExecuteStatus::TRANSPORT_OPEN_FAILED;
            result.message = "transport open failed: " + error;
            return result;
        }

        ExecuteStatus lastFailureStatus = ExecuteStatus::NO_ACK_RECEIVED;
        std::string lastFailureMessage;

        for (int n = 0; n < options.repeatCount; ++n)
        {
            if (!transport->Send(requestFrame, &error))
            {
                result.status = ExecuteStatus::SEND_FAILED;
                result.message = "send failed: " + error;
                transport->Close();
                return result;
            }

            if (!options.waitForAck)
            {
                continue;
            }

            std::vector<uint8_t> response;
            if (!transport->Receive(response, options.timeoutMs, &error))
            {
                lastFailureStatus = ExecuteStatus::RECEIVE_FAILED;
                lastFailureMessage = "receive failed or timed out (attempt " + std::to_string(n + 1) +
                                     "/" + std::to_string(options.repeatCount) + "): " + error;
            }
            else
            {
                result.lastResponseFrame = response;

                SIYIPacket decoded;
                if (!camera_->DecodeFrame(response, decoded, &error))
                {
                    lastFailureStatus = ExecuteStatus::DECODE_FAILED;
                    lastFailureMessage = "response decode failed: " + error;
                }
                else if (!AckPolicy::IsExpectedAckCmdIdForRequest(decoded.cmdId, requestCmdId))
                {
                    lastFailureStatus = ExecuteStatus::UNEXPECTED_ACK;
                    lastFailureMessage = "received unrelated response cmd_id";
                }
                else
                {
                    result.receivedAck = true;
                    result.decodedPacket = decoded;

                    TM::TelemetryMessage tmMessage;
                    if (camera_->DecodeTelemetryPacket(decoded, tmMessage, &error))
                    {
                        result.telemetry = tmMessage;
                    }
                    else
                    {
                        result.message = "typed telemetry decode skipped: " + error;
                    }

                    break;
                }
            }

            if (n + 1 < options.repeatCount && options.periodMs > 0)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(options.periodMs));
            }
        }

        transport->Close();

        if (!options.waitForAck)
        {
            result.status = ExecuteStatus::OK;
            result.message = "command sent without ACK request";
            return result;
        }

        if (result.receivedAck)
        {
            result.status = ExecuteStatus::OK;
            result.message = "acknowledged";
            return result;
        }

        result.status = lastFailureStatus;
        result.message = lastFailureMessage.empty() ? "no ACK received" : lastFailureMessage;
        return result;
    }
} // namespace SIYI