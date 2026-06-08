#include "doctest.h"

#include "../include/siyi/camera/models/optical_zoom_camera_model.hpp"

namespace SIYI
{
    class OpticalZoomTestBaseCamera final : public OpticalZoomCameraModel
    {
    public:
        explicit OpticalZoomTestBaseCamera(
            std::shared_ptr<TelecommandSession> session = std::make_shared<TelecommandSession>())
            : OpticalZoomCameraModel(-90, 25, -270, 270, -360, 360, 0, 180, 0, 10, session){
        }
    };

    TEST_SUITE("SIYI Optical Zoom Camera")
    {
        
        TEST_CASE("OpticalZoomCameraModel constructor initializes with correct limits")
        {
            OpticalZoomTestBaseCamera camera;

            CHECK(camera.GetPitchMin() == -90);
            CHECK(camera.GetPitchMax() == 25);
            CHECK(camera.GetYawMin() == -270);
            CHECK(camera.GetYawMax() == 270);
            CHECK(camera.GetRollMin() == -360);
            CHECK(camera.GetRollMax() == 360);
            CHECK(camera.GetZoomMax() == 180);
            CHECK(camera.GetZoomMin() == 0);
            CHECK(camera.GetZoomOpticalMin() == 0);
            CHECK(camera.GetZoomOpticalMax() == 10);
        }

        TEST_CASE("AutoFocus encodes expected payload")
        {
            OpticalZoomTestBaseCamera camera;
            const auto frame = camera.AutoFocus(100, 200);

            SIYIPacket packet;
            REQUIRE(camera.DecodeFrame(frame, packet));

            CHECK(packet.ctrl == static_cast<uint8_t>(ControlFlag::NEED_ACK));
            CHECK(packet.cmdId == static_cast<uint8_t>(CommandId::AUTO_FOCUS));
            CHECK(packet.data == std::vector<uint8_t>({0x01, 0x00, 0x64, 0x00, 0xC8}));
        }

        TEST_CASE("ManualFocus encodes expected payload")
        {
            OpticalZoomTestBaseCamera camera;
            const auto frame = camera.SetManualFocus(ManualFocusDirection::LONG_SHOT);

            SIYIPacket packet;
            REQUIRE(camera.DecodeFrame(frame, packet));

            CHECK(packet.ctrl == static_cast<uint8_t>(ControlFlag::NEED_ACK));
            CHECK(packet.cmdId == static_cast<uint8_t>(CommandId::MANUAL_FOCUS));
            CHECK(packet.data == std::vector<uint8_t>({0x01}));
        }

        TEST_CASE("ManualZoom encodes expected payload")
        {
            OpticalZoomTestBaseCamera camera;
            const auto frame = camera.SetManualZoom(ManualZoomDirection::ZOOM_OUT);

            SIYIPacket packet;
            REQUIRE(camera.DecodeFrame(frame, packet));

            CHECK(packet.ctrl == static_cast<uint8_t>(ControlFlag::NEED_ACK));
            CHECK(packet.cmdId == static_cast<uint8_t>(CommandId::ZOOM));
            CHECK(packet.data == std::vector<uint8_t>({0xFF}));
        }

        TEST_CASE("SetAbsoluteZoom int overload encodes expected payload")
        {
            OpticalZoomTestBaseCamera camera;
            const auto frame = camera.SetAbsoluteZoom(4);

            SIYIPacket packet;
            REQUIRE(camera.DecodeFrame(frame, packet));

            CHECK(packet.ctrl == static_cast<uint8_t>(ControlFlag::NEED_ACK));
            CHECK(packet.cmdId == static_cast<uint8_t>(CommandId::ABSOLUTE_ZOOM));
            CHECK(packet.data == std::vector<uint8_t>({0x04, 0x00}));
        }

        TEST_CASE("SetAbsoluteZoom float overload converts and clamps")
        {
            OpticalZoomTestBaseCamera camera;

            const auto frameRounded = camera.SetAbsoluteZoom(4.5F);
            const auto frameClamped = camera.SetAbsoluteZoom(231.2F);
            const auto frameClampedLow = camera.SetAbsoluteZoom(-5.0F);
            const auto frameBoundary = camera.SetAbsoluteZoom(10.0F);

            SIYIPacket roundedPacket;
            SIYIPacket clampedPacket;
            SIYIPacket clampedLowPacket;
            SIYIPacket boundaryPacket;
            REQUIRE(camera.DecodeFrame(frameRounded, roundedPacket));
            REQUIRE(camera.DecodeFrame(frameClamped, clampedPacket));
            REQUIRE(camera.DecodeFrame(frameClampedLow, clampedLowPacket));
            REQUIRE(camera.DecodeFrame(frameBoundary, boundaryPacket));

            CHECK(roundedPacket.data == std::vector<uint8_t>({0x04, 0x05}));
            CHECK(clampedPacket.data == std::vector<uint8_t>({0xB4, 0x00}));
            CHECK(clampedLowPacket.data == std::vector<uint8_t>({0x00, 0x00}));
            CHECK(boundaryPacket.data == std::vector<uint8_t>({0x0A, 0x00}));
        }

        TEST_CASE("AcquireZoomLevel encodes expected payload")
        {
            OpticalZoomTestBaseCamera camera;
            const auto frame = camera.AcquireZoomLevel();

            SIYIPacket packet;
            REQUIRE(camera.DecodeFrame(frame, packet));

            CHECK(packet.ctrl == static_cast<uint8_t>(ControlFlag::NEED_ACK));
            CHECK(packet.cmdId == static_cast<uint8_t>(CommandId::ACQUIRE_ZOOM_LEVEL));
            CHECK(packet.data.empty());
        }


        TEST_CASE("DecodeTelemetryPacket correctly decodes ACQUIRE_ZOOM_LEVEL response")
        {
            OpticalZoomTestBaseCamera camera;

            SIYIPacket packet;
            packet.cmdId = static_cast<uint8_t>(CommandId::ACQUIRE_ZOOM_LEVEL);
            packet.data = {0x05, 0x03}; // zoom level 5.3
            packet.dataLen = static_cast<uint16_t>(packet.data.size());

            TM::TelemetryMessage message;
            std::string error;
            REQUIRE(camera.DecodeTelemetryPacket(packet, message, &error));

            const auto* zoomLevelMsg = std::get_if<TM::AcquireZoomLevel>(&message);
            REQUIRE(zoomLevelMsg != nullptr);
            CHECK(zoomLevelMsg->GetZoom() == doctest::Approx(5.3));
        }

        TEST_CASE("DecodeTelemetryPacket returns false for ACQUIRE_ZOOM_LEVEL with invalid data length")
        {
            OpticalZoomTestBaseCamera camera;

            SIYIPacket packet;
            packet.cmdId = static_cast<uint8_t>(CommandId::ACQUIRE_ZOOM_LEVEL);
            packet.data = {0x05}; // invalid data length
            packet.dataLen = static_cast<uint16_t>(packet.data.size());

            TM::TelemetryMessage message;
            std::string error;
            CHECK_FALSE(camera.DecodeTelemetryPacket(packet, message, &error));
            CHECK(error == "Invalid data length for AcquireZoomLevel telemetry packet. Expected 2 bytes, got 1 bytes.");
        }

        TEST_CASE("DecodeTelemetryPacket returns false for unknown cmdId")
        {
            OpticalZoomTestBaseCamera camera;

            SIYIPacket packet;
            packet.cmdId = 0xFF; // unknown command ID
            packet.data = {0x00};
            packet.dataLen = static_cast<uint16_t>(packet.data.size());

            TM::TelemetryMessage message;
            std::string error;
            CHECK_FALSE(camera.DecodeTelemetryPacket(packet, message, &error));
            CHECK(error == "Unsupported telemetry packet with cmdId: 0xFF");
        }
    }
} // namespace SIYI
