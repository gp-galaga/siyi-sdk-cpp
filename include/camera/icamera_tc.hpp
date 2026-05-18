#ifndef SIYI_I_CAMERA_TC_HPP
#define SIYI_I_CAMERA_TC_HPP

#include "protocol/tc_parameter.hpp"
#include "protocol/tm_parameters.hpp"

#include <atomic>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace SIYI
{
    struct SIYIPacket;

    class TelecommandSession
    {
    public:
        uint16_t NextSeq() noexcept
        {
            return nextSeq_.fetch_add(1, std::memory_order_relaxed);
        }

        void ResetSeq(uint16_t nextSeq = 0) noexcept
        {
            nextSeq_.store(nextSeq, std::memory_order_relaxed);
        }

        uint16_t PeekNextSeq() const noexcept
        {
            return nextSeq_.load(std::memory_order_relaxed);
        }

    private:
        std::atomic<uint16_t> nextSeq_ {0};
    };

    class ITCCamera
    {
    public:
        explicit ITCCamera(
            std::shared_ptr<TelecommandSession> session = std::make_shared<TelecommandSession>())
            : session_(std::move(session))
        {
        }

        virtual ~ITCCamera() = default;
        
        virtual std::vector<uint8_t> SetManualZoom(
            ManualZoomDirection direction) const = 0;

        virtual std::vector<uint8_t> StartRotation(
            int8_t yawSpeed,
            int8_t pitchSpeed) const = 0;

        virtual std::vector<uint8_t> SetGimbalAngle(
            int16_t yaw,
            int16_t pitch) const = 0;

        virtual std::vector<uint8_t> StopRotation() const = 0;

        virtual std::vector<uint8_t> ControlPhotoRecord(
            PhotoRecordFunction funcType) const = 0;

        virtual std::vector<uint8_t> BuildCustomCommand(
            uint8_t cmdId,
            const std::vector<uint8_t>& payload,
            bool needAck) const = 0;

        virtual bool DecodeFrame(
            const std::vector<uint8_t>& frame,
            SIYIPacket& outPacket,
            std::string* error = nullptr) const = 0;

        virtual bool DecodeTelemetryPacket(
            const SIYIPacket& packet,
            TM::TelemetryMessage& outMessage,
            std::string* error = nullptr) const = 0;

        virtual bool DecodeTelemetryFrame(
            const std::vector<uint8_t>& frame,
            TM::TelemetryMessage& outMessage,
            std::string* error = nullptr) const = 0;

        void ResetSeq(uint16_t nextSeq = 0) const noexcept
        {
            session_->ResetSeq(nextSeq);
        }

        uint16_t PeekNextSeq() const noexcept
        {
            return session_->PeekNextSeq();
        }

        std::shared_ptr<TelecommandSession> GetSession() const noexcept
        {
            return session_;
        }

    protected:
        uint16_t IncrementSeq() const noexcept
        {
            return session_->NextSeq();
        }

    private:
        std::shared_ptr<TelecommandSession> session_;

    };
} // namespace SIYI

#endif
