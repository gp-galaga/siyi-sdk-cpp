#ifndef OPTICAL_ZOOM_CAMERA_MODEL_HPP
#define OPTICAL_ZOOM_CAMERA_MODEL_HPP

#include "shared_camera_model.hpp"

namespace SIYI
{
    struct OpticalZoomTechnicalSpecs
    {
        float opticalZoomMaxX {0.0F};
        float hybridZoomMaxX {0.0F};
        float focalLengthMinMm {0.0F};
        float focalLengthMaxMm {0.0F};
        float apertureMinF {0.0F};
        float apertureMaxF {0.0F};
        float fovDiagonalMinZoomDeg {0.0F};
        float fovHorizontalMinZoomDeg {0.0F};
        float fovDiagonalMaxOpticalZoomDeg {0.0F};
        float fovHorizontalMaxOpticalZoomDeg {0.0F};
    };

    class OpticalZoomCameraModel : public SharedCameraModel
    {
    public:
        OpticalZoomCameraModel(int16_t pitchMin, int16_t pitchMax,
                               int16_t yawMin, int16_t yawMax,
                               int16_t rollMin, int16_t rollMax,
                               int16_t zoomMin, int16_t zoomMax,
                               int16_t zoomOpticalMin, int16_t zoomOpticalMax,
                               std::shared_ptr<TelecommandSession> session = std::make_shared<TelecommandSession>(),
                               const CameraTechnicalSpecs& technicalSpecs = CameraTechnicalSpecs(),
                               const OpticalZoomTechnicalSpecs& opticalTechnicalSpecs = OpticalZoomTechnicalSpecs());
        ~OpticalZoomCameraModel() override = default;

        std::vector<uint8_t> SetManualZoom(ManualZoomDirection direction) const override;

        std::vector<uint8_t> AutoFocus(uint16_t x_coord, uint16_t y_coord) const override;

        std::vector<uint8_t> SetManualFocus(ManualFocusDirection direction) const override;

        std::vector<uint8_t> SetAbsoluteZoom(float zoomValue) const override;
        std::vector<uint8_t> SetAbsoluteZoom(int zoomValue) const override;

        std::vector<uint8_t> AcquireZoomLevel() const;
        
        int16_t GetZoomOpticalMin() const noexcept { return zoom_optical_min_; }
        int16_t GetZoomOpticalMax() const noexcept { return zoom_optical_max_; }
        const OpticalZoomTechnicalSpecs& GetOpticalTechnicalSpecs() const noexcept { return optical_technical_specs_; }

        bool DecodeTelemetryPacket(
            const SIYIPacket& packet,
            TM::TelemetryMessage& outMessage,
            std::string* error = nullptr) const override;
        
        protected:
            std::vector<uint8_t> SetAbsoluteZoomRaw(uint8_t int_zoomValue, uint8_t frac_zoomValue) const;
        
        private:
            int16_t zoom_optical_min_ {0};
            int16_t zoom_optical_max_ {0};
            OpticalZoomTechnicalSpecs optical_technical_specs_ {};
    };
} // namespace SIYI

#endif // OPTICAL_ZOOM_CAMERA_MODEL_HPP