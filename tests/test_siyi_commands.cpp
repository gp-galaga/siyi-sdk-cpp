#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include "../include/camera/izr_camera.hpp"

namespace SIYI
{
    TEST_SUITE("SIYI Command Encoding")
    {
        TEST_CASE("Photo-record command keeps payload and uses no-ACK control byte")
        {
            ZR30 camera;
            const auto frame = camera.TakePicture();

            SIYIPacket packet;
            REQUIRE(camera.DecodeFrame(frame, packet));
            CHECK(packet.ctrl == static_cast<uint8_t>(ControlFlag::NO_ACK));
            CHECK(packet.cmdId == static_cast<uint8_t>(CommandId::PHOTO_RECORD));
            REQUIRE(packet.data.size() == 1);
            CHECK(packet.data[0] == static_cast<uint8_t>(PhotoRecordFunction::TAKE_PICTURE));
        }

        TEST_CASE("Control byte reflects needAck flag")
        {
            ZR30 camera;

            const auto rotateFrameAck = camera.StartRotation(100, 100, true);
            const auto rotateFrameNoAck = camera.StartRotation(100, 100, false);
            const auto zoomFrameAck = camera.SetAbsoluteZoom(3, true);
            const auto zoomFrameNoAck = camera.SetAbsoluteZoom(3, false);

            SIYIPacket rotateAck;
            SIYIPacket rotateNoAck;
            SIYIPacket zoomAck;
            SIYIPacket zoomNoAck;

            REQUIRE(camera.DecodeFrame(rotateFrameAck, rotateAck));
            REQUIRE(camera.DecodeFrame(rotateFrameNoAck, rotateNoAck));
            REQUIRE(camera.DecodeFrame(zoomFrameAck, zoomAck));
            REQUIRE(camera.DecodeFrame(zoomFrameNoAck, zoomNoAck));

            CHECK(rotateAck.ctrl == static_cast<uint8_t>(ControlFlag::NEED_ACK));
            CHECK(rotateNoAck.ctrl == static_cast<uint8_t>(ControlFlag::NO_ACK));
            CHECK(zoomAck.ctrl == static_cast<uint8_t>(ControlFlag::NEED_ACK));
            CHECK(zoomNoAck.ctrl == static_cast<uint8_t>(ControlFlag::NO_ACK));
        }

        TEST_CASE("Sequence increments on each encode")
        {
            ZR30 camera;

            const auto firstFrame = camera.TakePicture();
            const auto secondFrame = camera.StartStopRecording();
            const auto thirdFrame = camera.SetAbsoluteZoom(1, true);

            SIYIPacket firstPacket;
            SIYIPacket secondPacket;
            SIYIPacket thirdPacket;

            REQUIRE(camera.DecodeFrame(firstFrame, firstPacket));
            REQUIRE(camera.DecodeFrame(secondFrame, secondPacket));
            REQUIRE(camera.DecodeFrame(thirdFrame, thirdPacket));

            CHECK(firstPacket.seq == 0);
            CHECK(secondPacket.seq == 1);
            CHECK(thirdPacket.seq == 2);
        }

        TEST_CASE("ResetSeq restarts numbering from the requested value")
        {
            ZR30 camera;

            const auto firstFrame = camera.TakePicture();
            const auto secondFrame = camera.StartStopRecording();
            camera.ResetSeq();
            const auto thirdFrame = camera.ToggleHDR();

            SIYIPacket firstPacket;
            SIYIPacket secondPacket;
            SIYIPacket thirdPacket;

            REQUIRE(camera.DecodeFrame(firstFrame, firstPacket));
            REQUIRE(camera.DecodeFrame(secondFrame, secondPacket));
            REQUIRE(camera.DecodeFrame(thirdFrame, thirdPacket));

            CHECK(firstPacket.seq == 0);
            CHECK(secondPacket.seq == 1);
            CHECK(thirdPacket.seq == 0);
            CHECK(camera.PeekNextSeq() == 1);
        }

        TEST_CASE("Shared telecommand session advances sequence across cameras")
        {
            auto sharedSession = std::make_shared<TelecommandSession>();
            ZR30 firstCamera(sharedSession);
            ZR30 secondCamera(sharedSession);

            const auto firstFrame = firstCamera.TakePicture();
            const auto secondFrame = secondCamera.StartStopRecording();

            SIYIPacket firstPacket;
            SIYIPacket secondPacket;

            REQUIRE(firstCamera.DecodeFrame(firstFrame, firstPacket));
            REQUIRE(secondCamera.DecodeFrame(secondFrame, secondPacket));

            CHECK(firstPacket.seq == 0);
            CHECK(secondPacket.seq == 1);
            CHECK(sharedSession->PeekNextSeq() == 2);
        }

        TEST_CASE("Convenience methods use same payload as base method")
        {
            ZR30 camera;

            auto pictureDirect = camera.ControlPhotoRecord(
                PhotoRecordFunction::TAKE_PICTURE);
            auto pictureConv = camera.TakePicture();

            auto recordDirect = camera.ControlPhotoRecord(
                PhotoRecordFunction::START_STOP_RECORDING);
            auto recordConv = camera.StartStopRecording();

            auto hdrDirect = camera.ControlPhotoRecord(
                PhotoRecordFunction::TOGGLE_HDR);
            auto hdrConv = camera.ToggleHDR();

            SIYIPacket pictureDirectPacket;
            SIYIPacket pictureConvPacket;
            SIYIPacket recordDirectPacket;
            SIYIPacket recordConvPacket;
            SIYIPacket hdrDirectPacket;
            SIYIPacket hdrConvPacket;

            REQUIRE(camera.DecodeFrame(pictureDirect, pictureDirectPacket));
            REQUIRE(camera.DecodeFrame(pictureConv, pictureConvPacket));
            REQUIRE(camera.DecodeFrame(recordDirect, recordDirectPacket));
            REQUIRE(camera.DecodeFrame(recordConv, recordConvPacket));
            REQUIRE(camera.DecodeFrame(hdrDirect, hdrDirectPacket));
            REQUIRE(camera.DecodeFrame(hdrConv, hdrConvPacket));

            CHECK(pictureDirectPacket.ctrl == pictureConvPacket.ctrl);
            CHECK(pictureDirectPacket.cmdId == pictureConvPacket.cmdId);
            CHECK(pictureDirectPacket.data == pictureConvPacket.data);

            CHECK(recordDirectPacket.ctrl == recordConvPacket.ctrl);
            CHECK(recordDirectPacket.cmdId == recordConvPacket.cmdId);
            CHECK(recordDirectPacket.data == recordConvPacket.data);

            CHECK(hdrDirectPacket.ctrl == hdrConvPacket.ctrl);
            CHECK(hdrDirectPacket.cmdId == hdrConvPacket.cmdId);
            CHECK(hdrDirectPacket.data == hdrConvPacket.data);
        }

        TEST_CASE("Common commands encode expected command IDs and payloads")
        {
            ZR30 camera;

            const auto fwFrame = camera.AcquireFirmwareVersion();
            const auto hwFrame = camera.AcquireHardwareId();
            const auto autoFocusFrame = camera.AutoFocus(100, 200);
            const auto centerFrame = camera.Center();
            const auto absoluteZoomFrame = camera.SetAbsoluteZoom(12.3F);
            const auto softRestartFrame = camera.SoftRestart(true, false);

            SIYIPacket fwPacket;
            SIYIPacket hwPacket;
            SIYIPacket autoFocusPacket;
            SIYIPacket centerPacket;
            SIYIPacket absoluteZoomPacket;
            SIYIPacket softRestartPacket;

            REQUIRE(camera.DecodeFrame(fwFrame, fwPacket));
            REQUIRE(camera.DecodeFrame(hwFrame, hwPacket));
            REQUIRE(camera.DecodeFrame(autoFocusFrame, autoFocusPacket));
            REQUIRE(camera.DecodeFrame(centerFrame, centerPacket));
            REQUIRE(camera.DecodeFrame(absoluteZoomFrame, absoluteZoomPacket));
            REQUIRE(camera.DecodeFrame(softRestartFrame, softRestartPacket));

            CHECK(fwPacket.ctrl == static_cast<uint8_t>(ControlFlag::NEED_ACK));
            CHECK(fwPacket.cmdId == static_cast<uint8_t>(CommandId::ACQUIRE_FW_VER));
            CHECK(fwPacket.data.empty());

            CHECK(hwPacket.ctrl == static_cast<uint8_t>(ControlFlag::NEED_ACK));
            CHECK(hwPacket.cmdId == static_cast<uint8_t>(CommandId::ACQUIRE_HW_ID));
            CHECK(hwPacket.data.empty());

            CHECK(autoFocusPacket.ctrl == static_cast<uint8_t>(ControlFlag::NEED_ACK));
            CHECK(autoFocusPacket.cmdId == static_cast<uint8_t>(CommandId::AUTO_FOCUS));
            CHECK(autoFocusPacket.data == std::vector<uint8_t>({0x01, 0x00, 0x64, 0x00, 0xC8}));

            CHECK(centerPacket.ctrl == static_cast<uint8_t>(ControlFlag::NEED_ACK));
            CHECK(centerPacket.cmdId == static_cast<uint8_t>(CommandId::CENTER));
            CHECK(centerPacket.data == std::vector<uint8_t>({0x01}));

            CHECK(absoluteZoomPacket.ctrl == static_cast<uint8_t>(ControlFlag::NEED_ACK));
            CHECK(absoluteZoomPacket.cmdId == static_cast<uint8_t>(CommandId::ABSOLUTE_ZOOM));
            CHECK(absoluteZoomPacket.data == std::vector<uint8_t>({12, 3}));

            CHECK(softRestartPacket.ctrl == static_cast<uint8_t>(ControlFlag::NEED_ACK));
            CHECK(softRestartPacket.cmdId == static_cast<uint8_t>(CommandId::SOFT_RESTART));
            CHECK(softRestartPacket.data == std::vector<uint8_t>({1, 0}));
        }

        TEST_CASE("SetUtcTime encodes uint64 payload")
        {
            ZR30 camera;

            constexpr uint64_t kUtcUs = 0x0102030405060708ULL;
            const auto frame = camera.SetUtcTime(kUtcUs);

            SIYIPacket packet;
            REQUIRE(camera.DecodeFrame(frame, packet));

            CHECK(packet.ctrl == static_cast<uint8_t>(ControlFlag::NEED_ACK));
            CHECK(packet.cmdId == static_cast<uint8_t>(CommandId::SET_UTC_TIME));
            REQUIRE(packet.data.size() == 8);
            CHECK(packet.data == std::vector<uint8_t>({0x08, 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01}));
        }

        TEST_CASE("SetUtcTime encodes chrono payload")
        {
            ZR30 camera;

            constexpr std::chrono::microseconds kUtcUs(0x0102030405060708ULL);
            const auto frame = camera.SetUtcTime(kUtcUs);

            SIYIPacket packet;
            REQUIRE(camera.DecodeFrame(frame, packet));

            CHECK(packet.ctrl == static_cast<uint8_t>(ControlFlag::NEED_ACK));
            CHECK(packet.cmdId == static_cast<uint8_t>(CommandId::SET_UTC_TIME));
            REQUIRE(packet.data.size() == 8);
            CHECK(packet.data == std::vector<uint8_t>({0x08, 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01}));
        }

        TEST_CASE("SoftRestart encodes expected payload")
        {
            ZR30 camera;

            const auto frame = camera.SoftRestart(true, true);

            SIYIPacket packet;
            REQUIRE(camera.DecodeFrame(frame, packet));

            CHECK(packet.ctrl == static_cast<uint8_t>(ControlFlag::NEED_ACK));
            CHECK(packet.cmdId == static_cast<uint8_t>(CommandId::SOFT_RESTART));
            REQUIRE(packet.data.size() == 2);
            CHECK(packet.data == std::vector<uint8_t>({1, 1}));
        }

        TEST_CASE("Decode typed gimbal attitude telemetry")
        {
            ZR30 camera;

            SIYIPacket packet;
            packet.ctrl = MakeDeviceAckControlByte();
            packet.seq = 42;
            packet.cmdId = static_cast<uint8_t>(CommandId::ACQUIRE_GIMBAL_ATT);
            packet.data = {
                0x7B, 0x00, // yaw = 123
                0xCE, 0xFF, // pitch = -50
                0x00, 0x00, // roll = 0
                0x0F, 0x00, // yaw_velocity = 15
                0xFB, 0xFF, // pitch_velocity = -5
                0x2C, 0x01  // roll_velocity = 300
            };
            packet.dataLen = static_cast<uint16_t>(packet.data.size());

            TM::TelemetryMessage message;
            std::string error;
            REQUIRE(camera.DecodeTelemetryPacket(packet, message, &error));

            const auto* att = std::get_if<TM::GimbalAttitude>(&message);
            REQUIRE(att != nullptr);
            CHECK(att->yaw == 123);
            CHECK(att->pitch == -50);
            CHECK(att->roll == 0);
            CHECK(att->yawVelocity == 15);
            CHECK(att->pitchVelocity == -5);
            CHECK(att->rollVelocity == 300);
            CHECK(att->YawDeg() == doctest::Approx(12.3));
            CHECK(att->PitchDeg() == doctest::Approx(-5.0));
            CHECK(att->RollVelocityDegPerSec() == doctest::Approx(30.0));
        }

        TEST_CASE("Decode typed gimbal configuration telemetry")
        {
            ZR30 camera;

            SIYIPacket packet;
            packet.ctrl = MakeDeviceAckControlByte();
            packet.seq = 7;
            packet.cmdId = static_cast<uint8_t>(CommandId::ACQUIRE_GIMBAL_CONFIGURATION);
            packet.data = {
                0x00,
                0x01,
                0x00,
                0x02,
                0x01,
                0x02,
                0x00
            };
            packet.dataLen = static_cast<uint16_t>(packet.data.size());

            TM::TelemetryMessage message;
            std::string error;
            REQUIRE(camera.DecodeTelemetryPacket(packet, message, &error));

            const auto* config = std::get_if<TM::GimbalConfiguration>(&message);
            REQUIRE(config != nullptr);
            CHECK(config->reserved0 == 0x00);
            CHECK(config->hdrStatus == 0x01);
            CHECK(config->reserved1 == 0x00);
            CHECK(config->recordStatus == 0x02);
            CHECK(config->gimbalMotionMode == 0x01);
            CHECK(config->gimbalMountingMethod == 0x02);
            CHECK(config->video_hdmi_or_cvbs == 0x00);
        }

        TEST_CASE("Reject invalid gimbal configuration payload length")
        {
            ZR30 camera;

            SIYIPacket packet;
            packet.ctrl = MakeDeviceAckControlByte();
            packet.cmdId = static_cast<uint8_t>(CommandId::ACQUIRE_GIMBAL_CONFIGURATION);
            packet.data = {0x00, 0x01, 0x00, 0x02, 0x01, 0x00};
            packet.dataLen = static_cast<uint16_t>(packet.data.size());

            TM::TelemetryMessage message;
            std::string error;
            CHECK_FALSE(camera.DecodeTelemetryPacket(packet, message, &error));
            CHECK(error.find("7 bytes") != std::string::npos);
        }

        TEST_CASE("Enum helper method maps raw value to enum")
        {
            TM::GimbalWorkingMode mode{};
            mode.gimbalWorkingMode = 1;

            CHECK(mode.AsGimbalWorkingMode() == TM::GimbalWorkingModeEnum::FOLLOW_MODE);
        }

        TEST_CASE("TC metadata reports command scope and camera support")
        {
            CHECK(GetCommandScope(CommandId::AUTO_FOCUS) == CommandScope::ZOOM_CAMERA);
            CHECK(GetCommandScope(CommandId::ACQUIRE_FW_VER) == CommandScope::COMMON);

            CHECK(IsCommandSupportedByCamera(CommandId::AUTO_FOCUS, CameraModel::ZR10));
            CHECK(IsCommandSupportedByCamera(CommandId::AUTO_FOCUS, CameraModel::ZR30));
            CHECK(IsCommandSupportedByCamera(CommandId::AUTO_FOCUS, CameraModel::ZT30));
            CHECK_FALSE(IsCommandSupportedByCamera(CommandId::AUTO_FOCUS, CameraModel::A8_MINI));

            CHECK(IsCommandSupportedByCamera(CommandId::CENTER, CameraModel::A8_MINI));
        }
    }
} // namespace SIYI
