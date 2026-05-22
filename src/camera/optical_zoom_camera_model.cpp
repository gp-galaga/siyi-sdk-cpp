#include "../../include/siyi/camera/models/optical_zoom_camera_model.hpp"

namespace SIYI
{

        OpticalZoomCameraModel::OpticalZoomCameraModel(
            int16_t pitchMin, int16_t pitchMax,
            int16_t yawMin, int16_t yawMax,
            int16_t rollMin, int16_t rollMax,
            int16_t zoomMin, int16_t zoomMax,
            int16_t zoomOpticalMin, int16_t zoomOpticalMax,
            std::shared_ptr<TelecommandSession> session,
            const CameraTechnicalSpecs &technicalSpecs,
            const OpticalZoomTechnicalSpecs &opticalTechnicalSpecs)
            : SharedCameraModel(pitchMin, pitchMax, yawMin, yawMax, rollMin, rollMax, zoomMin, zoomMax, session, technicalSpecs),
              optical_technical_specs_(opticalTechnicalSpecs)
        {
            zoom_optical_min_ = zoomOpticalMin;
            zoom_optical_max_ = zoomOpticalMax;
        }

    std::vector<uint8_t> OpticalZoomCameraModel::SetManualZoom(const ManualZoomDirection direction) const
    {
        const std::vector<uint8_t> payload = {static_cast<uint8_t>(direction)};
        return BuildPacket(
            static_cast<uint8_t>(CommandId::ZOOM),
            payload,
            ControlFlag::NEED_ACK);
    }

    std::vector<uint8_t> OpticalZoomCameraModel::AutoFocus(uint16_t x_coord, uint16_t y_coord) const
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

    std::vector<uint8_t> OpticalZoomCameraModel::SetManualFocus(const ManualFocusDirection direction) const
    {
        const std::vector<uint8_t> payload = {static_cast<uint8_t>(direction)};
        return BuildPacket(
            static_cast<uint8_t>(CommandId::MANUAL_FOCUS),
            payload,
            ControlFlag::NEED_ACK);
    }

    std::vector<uint8_t> OpticalZoomCameraModel::SetAbsoluteZoomRaw(uint8_t int_zoomValue, uint8_t frac_zoomValue) const
    {
        return BuildPacket(
            static_cast<uint8_t>(CommandId::ABSOLUTE_ZOOM),
            {int_zoomValue, frac_zoomValue},
            ControlFlag::NEED_ACK);
    }

    std::vector<uint8_t> OpticalZoomCameraModel::SetAbsoluteZoom(const int zoomValue) const
    {
        return SetAbsoluteZoomRaw(static_cast<uint8_t>(zoomValue), 0);
    }

    std::vector<uint8_t> OpticalZoomCameraModel::SetAbsoluteZoom(float zoomValue) const{
        if (zoomValue < static_cast<float>(zoom_min_))
        {
            zoomValue = static_cast<float>(zoom_min_);
        }

        if(zoomValue > static_cast<float>(zoom_optical_max_))
        {
            logger_->PrintWarning("Absolute zoom value " + std::to_string(zoomValue) + " exceeds optical max of " + std::to_string(zoom_optical_max_) + ". Now using numerical zoom.");
            if (zoomValue > static_cast<float>(zoom_max_))
            {
                zoomValue = static_cast<float>(zoom_max_);
                logger_->PrintWarning("Absolute zoom value exceeds numerical max. Clamping to " + std::to_string(zoom_max_) + ".");
            }
        }

        
        uint8_t intPart = static_cast<uint8_t>(zoomValue);
        uint8_t fracPart = static_cast<uint8_t>((zoomValue - static_cast<float>(intPart)) * 10.0F + 0.5F);
        if (fracPart > 9)
        {
            fracPart = 9;
        }

        return OpticalZoomCameraModel::SetAbsoluteZoomRaw(intPart, fracPart);
    }
} // namespace SIYI