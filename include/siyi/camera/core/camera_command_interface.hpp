#ifndef SIYI_CAMERA_COMMAND_INTERFACE_HPP
#define SIYI_CAMERA_COMMAND_INTERFACE_HPP

#include "telecommand_session.hpp"
#include "../protocol/tc_parameter.hpp"

#include <cstdint>
#include <memory>
#include <vector>

namespace SIYI
{
    class ICommandCamera
    {
    public:
        explicit ICommandCamera(
            std::shared_ptr<TelecommandSession> session = std::make_shared<TelecommandSession>())
            : session_(std::move(session))
        {
        }

        virtual ~ICommandCamera() = default;

        virtual std::vector<uint8_t> BuildCustomCommand(
            uint8_t cmdId,
            const std::vector<uint8_t>& payload,
            bool needAck) const = 0;

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

#endif // SIYI_CAMERA_COMMAND_INTERFACE_HPP
