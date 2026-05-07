#include "../../include/camera/izr_camera.hpp"

namespace SIYI
{
    // std::vector<uint8_t> ZR30::SetGimbalFollowMode(
    //     const bool enabled,
    //     const bool needAck) const
    // {
    //     // Example ZR30-specific command: 0x20 with one-byte payload.
    //     const std::vector<uint8_t> payload = {static_cast<uint8_t>(enabled ? 0x01 : 0x00)};
    //     return BuildCustomCommand(0x20, payload, needAck);
    // }

    

    ZR30::ZR30(std::shared_ptr<TelecommandSession> session)
        : SIYICameraBase(std::move(session))
    {
        pitchLimit_ = {-90, 25};    // -90 to +90 degrees in centi-degrees
        yawLimit_ = {-270, 270};    // -270 to +270 degrees in centi-degrees
        zoomLimit_ = {0, 180};      // 0% to 180% zoom in hundredths
        rollLimit_ = {45, 45};      // -360 to +360 degrees in centi-degrees
        AngleLimit zoomOpticalLimit_ = {0, 30};      // 0% to 30% optical zoom in hundredths
        AngleLimit zoomDigitalLimit_ = {30, 180};     // 0% to 150% digital zoom in hundredths
    }
} // namespace SIYI