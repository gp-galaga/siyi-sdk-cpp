#include "../../include/camera/models/zr30_camera.hpp"

namespace SIYI
{
    std::vector<uint8_t> ZR30::SetManualZoom(const ManualZoomDirection direction) const
    {
        const std::vector<uint8_t> payload = {static_cast<uint8_t>(direction)};
        return BuildPacket(
            static_cast<uint8_t>(CommandId::ZOOM),
            payload,
            ControlFlag::NEED_ACK);
    }

    std::vector<uint8_t> ZR30::AutoFocus(uint16_t x_coord, uint16_t y_coord) const
    {
        std::vector<uint8_t> payload = {
            0x01, // AF start
            static_cast<uint8_t>((x_coord >> 8) & 0xFF),
            static_cast<uint8_t>(x_coord & 0xFF),
            static_cast<uint8_t>((y_coord >> 8) & 0xFF),
            static_cast<uint8_t>(y_coord & 0xFF)};
        return BuildPacket(
            static_cast<uint8_t>(CommandId::AUTO_FOCUS),
            payload,
            ControlFlag::NEED_ACK);
    }

    std::vector<uint8_t> ZR30::SetManualFocus(const ManualFocusDirection direction) const
    {
        const std::vector<uint8_t> payload = {static_cast<uint8_t>(direction)};
        return BuildPacket(
            static_cast<uint8_t>(CommandId::MANUAL_FOCUS),
            payload,
            ControlFlag::NEED_ACK);
    }

    std::vector<uint8_t> ZR30::SetAbsoluteZoom(uint8_t int_zoomValue, uint8_t frac_zoomValue) const
    {
        return BuildPacket(
            static_cast<uint8_t>(CommandId::ABSOLUTE_ZOOM),
            {int_zoomValue, frac_zoomValue},
            ControlFlag::NEED_ACK);
    }

    ZR30::ZR30(std::shared_ptr<TelecommandSession> session)
        : SIYICameraBase(std::move(session))
    {
        pitchLimit_ = {-90, 25};    // -90 to +90 degrees in centi-degrees
        yawLimit_ = {-270, 270};    // -270 to +270 degrees in centi-degrees
        zoomLimit_ = {0, 180};      // 0% to 180% zoom in hundredths
        rollLimit_ = {45, 45};      // -360 to +360 degrees in centi-degrees
        zoomOpticalLimit_ = {0, 30};      // 0% to 30% optical zoom in hundredths
    }

    std::vector<uint8_t> ZR30::SetAbsoluteZoom(float zoomValue) const{
        if (zoomValue < zoomOpticalLimit_.min)
        {
            zoomValue = zoomOpticalLimit_.min;
        }


        if (zoomValue > zoomOpticalLimit_.max)
        {
            zoomValue = zoomOpticalLimit_.max;
        }
        
        uint8_t intPart = static_cast<uint8_t>(zoomValue);
        uint8_t fracPart = static_cast<uint8_t>((zoomValue - static_cast<float>(intPart)) * 10.0F + 0.5F);
        if (fracPart > 9)
        {
            fracPart = 9;
        }

        return ZR30::SetAbsoluteZoom(intPart, fracPart);
    }
} // namespace SIYI