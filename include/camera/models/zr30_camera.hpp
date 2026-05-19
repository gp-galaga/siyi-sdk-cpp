#ifndef __ZR30_CAMERA_HPP__
#define __ZR30_CAMERA_HPP__

#include <cstdint>
#include <memory>
#include <vector>
#include "siyi_camera_base.hpp"

namespace SIYI
{
    // Example concrete camera model extending the shared command set.
    class ZR30 : public SIYICameraBase
    {
    public:
        explicit ZR30(std::shared_ptr<TelecommandSession> session = std::make_shared<TelecommandSession>());

        std::vector<uint8_t> SetManualZoom(ManualZoomDirection direction) const;

        std::vector<uint8_t> AutoFocus(uint16_t x_coord, uint16_t y_coord) const;

        std::vector<uint8_t> SetManualFocus(ManualFocusDirection direction) const;

        std::vector<uint8_t> SetAbsoluteZoom(uint8_t int_zoomValue, uint8_t frac_zoomValue) const;

        std::vector<uint8_t> SetAbsoluteZoom(float zoomValue) const;

    private:
        AngleLimit zoomOpticalLimit_ {};
    };
}

#endif // __ZR30_CAMERA_HPP__
