#include "../../../include/siyi/camera/factories/camera_model_factory.hpp"

namespace SIYI
{
    std::shared_ptr<SharedCameraModel> CreateCameraModel(
        const TM::GimbalModel kind,
        std::shared_ptr<TelecommandSession> session)
    {
        switch (kind)
        {
            case TM::GimbalModel::ZR30:
            default:
                return std::make_shared<ZR30>(std::move(session));
        }
    }
} // namespace SIYI