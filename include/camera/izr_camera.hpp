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
    };
}

#endif // __ZR30_CAMERA_HPP__