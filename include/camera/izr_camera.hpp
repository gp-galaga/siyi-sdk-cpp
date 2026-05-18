#ifndef __ZR30_CAMERA_HPP__
#define __ZR30_CAMERA_HPP__

#include <cstdint>
#include <memory>
#include <vector>
#include "ibase_camera.hpp"

namespace SIYI
{
    // Example concrete camera model extending the shared command set.
    class ZR30 : public SIYICameraBase
    {
    public:
        explicit ZR30(
            std::shared_ptr<TelecommandSession> session = std::make_shared<TelecommandSession>());

        // Example model-specific command wrapper.
        std::vector<uint8_t> SetGimbalFollowMode(bool enabled, bool needAck = false) const;

        std::vector<uint8_t> SetManualFocus(
            const ManualFocusDirection direction) const
        {
            const std::vector<uint8_t> payload = {static_cast<uint8_t>(direction)};
            return BuildPacket(
                static_cast<uint8_t>(CommandId::MANUAL_FOCUS),
                payload,
                ControlFlag::NEED_ACK);
        }
    };
}

#endif // __ZR30_CAMERA_HPP__