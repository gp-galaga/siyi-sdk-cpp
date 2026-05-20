#ifndef __ZR30_CAMERA_HPP__
#define __ZR30_CAMERA_HPP__

#include <cstdint>
#include <memory>
#include <vector>
#include "optical_zoom_camera_model.hpp"

namespace SIYI
{
    // Example concrete camera model extending the shared command set.
    class ZR30 : public OpticalZoomCameraModel
    {
        public:
            explicit ZR30(std::shared_ptr<TelecommandSession> session = std::make_shared<TelecommandSession>());
    };
}

#endif // __ZR30_CAMERA_HPP__
