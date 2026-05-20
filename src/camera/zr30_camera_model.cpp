#include "../../include/siyi/camera/models/zr30_camera_model.hpp"
#include <iostream>

namespace SIYI
{
    ZR30::ZR30(std::shared_ptr<TelecommandSession> session)
        : OpticalZoomCameraModel(-90, 25, -270, 270, 45, 45, 0, 180, 0, 30, session){
            std::cout << "ZR30 camera model initialized with specific limits and capabilities.\n";
        }
} // namespace SIYI