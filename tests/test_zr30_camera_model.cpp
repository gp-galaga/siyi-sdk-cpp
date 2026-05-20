#include "doctest.h"

#include "../include/siyi/camera/models/zr30_camera_model.hpp"

namespace SIYI
{
    TEST_SUITE("SIYI ZR30 Camera Model")
    {
        TEST_CASE("ZR30 constructor initializes with correct limits")
        {
            ZR30 camera;

            CHECK(camera.GetPitchMin() == -90);
            CHECK(camera.GetPitchMax() == 25);
            CHECK(camera.GetYawMin() == -270);
            CHECK(camera.GetYawMax() == 270);
            CHECK(camera.GetRollMin() == 45);
            CHECK(camera.GetRollMax() == 45);
            CHECK(camera.GetZoomOpticalMin() == 0);
            CHECK(camera.GetZoomOpticalMax() == 30);
            CHECK(camera.GetZoomMin() == 0);
            CHECK(camera.GetZoomMax() == 180);
        }

        TEST_CASE("ZR30 inherits optical zoom command implementations")
        {
            ZR30 camera;

            const auto autoFocusFrame = camera.AutoFocus(100, 200);
            const auto manualFocusFrame = camera.SetManualFocus(ManualFocusDirection::LONG_SHOT);
            const auto absZoomFrame = camera.SetAbsoluteZoom(4, 5);

            SIYIPacket autoFocusPacket;
            SIYIPacket manualFocusPacket;
            SIYIPacket absZoomPacket;

            REQUIRE(camera.DecodeFrame(autoFocusFrame, autoFocusPacket));
            REQUIRE(camera.DecodeFrame(manualFocusFrame, manualFocusPacket));
            REQUIRE(camera.DecodeFrame(absZoomFrame, absZoomPacket));

            CHECK(autoFocusPacket.cmdId == static_cast<uint8_t>(CommandId::AUTO_FOCUS));
            CHECK(autoFocusPacket.data == std::vector<uint8_t>({0x01, 0x00, 0x64, 0x00, 0xC8}));

            CHECK(manualFocusPacket.cmdId == static_cast<uint8_t>(CommandId::MANUAL_FOCUS));
            CHECK(manualFocusPacket.data == std::vector<uint8_t>({0x01}));

            CHECK(absZoomPacket.cmdId == static_cast<uint8_t>(CommandId::ABSOLUTE_ZOOM));
            CHECK(absZoomPacket.data == std::vector<uint8_t>({0x04, 0x05}));
        }
    };
} // namespace SIYI