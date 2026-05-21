#ifndef SIYI_CAMERA_MODEL_FACTORY_HPP
#define SIYI_CAMERA_MODEL_FACTORY_HPP

#include "../protocol/tm_parameters.hpp"
#include "../models/zr30_camera_model.hpp"

#include <memory>

namespace SIYI
{
    std::shared_ptr<SharedCameraModel> CreateCameraModel(
        TM::GimbalModel kind,
        std::shared_ptr<TelecommandSession> session = std::make_shared<TelecommandSession>());
} // namespace SIYI

#endif // SIYI_CAMERA_MODEL_FACTORY_HPP