#include "doctest.h"

#include "../include/camera/models/zr30_camera.hpp"

namespace SIYI
{
    TEST_SUITE("SIYI ZR30 Camera")
    {
        TEST_CASE("AutoFocus encodes expected payload")
        {
            ZR30 camera;
            const auto frame = camera.AutoFocus(100, 200);

            SIYIPacket packet;
            REQUIRE(camera.DecodeFrame(frame, packet));

            CHECK(packet.ctrl == static_cast<uint8_t>(ControlFlag::NEED_ACK));
            CHECK(packet.cmdId == static_cast<uint8_t>(CommandId::AUTO_FOCUS));
            CHECK(packet.data == std::vector<uint8_t>({0x01, 0x00, 0x64, 0x00, 0xC8}));
        }

        TEST_CASE("ManualFocus encodes expected payload")
        {
            ZR30 camera;
            const auto frame = camera.SetManualFocus(ManualFocusDirection::LONG_SHOT);

            SIYIPacket packet;
            REQUIRE(camera.DecodeFrame(frame, packet));

            CHECK(packet.ctrl == static_cast<uint8_t>(ControlFlag::NEED_ACK));
            CHECK(packet.cmdId == static_cast<uint8_t>(CommandId::MANUAL_FOCUS));
            CHECK(packet.data == std::vector<uint8_t>({0x01}));
        }

        TEST_CASE("ManualZoom encodes expected payload")
        {
            ZR30 camera;
            const auto frame = camera.SetManualZoom(ManualZoomDirection::ZOOM_OUT);

            SIYIPacket packet;
            REQUIRE(camera.DecodeFrame(frame, packet));

            CHECK(packet.ctrl == static_cast<uint8_t>(ControlFlag::NEED_ACK));
            CHECK(packet.cmdId == static_cast<uint8_t>(CommandId::ZOOM));
            CHECK(packet.data == std::vector<uint8_t>({0xFF}));
        }

        TEST_CASE("SetAbsoluteZoom int-frac overload encodes expected payload")
        {
            ZR30 camera;
            const auto frame = camera.SetAbsoluteZoom(4, 5);

            SIYIPacket packet;
            REQUIRE(camera.DecodeFrame(frame, packet));

            CHECK(packet.ctrl == static_cast<uint8_t>(ControlFlag::NEED_ACK));
            CHECK(packet.cmdId == static_cast<uint8_t>(CommandId::ABSOLUTE_ZOOM));
            CHECK(packet.data == std::vector<uint8_t>({0x04, 0x05}));
        }

        TEST_CASE("SetAbsoluteZoom float overload converts and clamps")
        {
            ZR30 camera;

            const auto frameRounded = camera.SetAbsoluteZoom(4.5F);
            const auto frameClamped = camera.SetAbsoluteZoom(31.2F);

            SIYIPacket roundedPacket;
            SIYIPacket clampedPacket;
            REQUIRE(camera.DecodeFrame(frameRounded, roundedPacket));
            REQUIRE(camera.DecodeFrame(frameClamped, clampedPacket));

            CHECK(roundedPacket.data == std::vector<uint8_t>({0x04, 0x05}));
            CHECK(clampedPacket.data == std::vector<uint8_t>({0x1E, 0x00}));
        }
    }
} // namespace SIYI
