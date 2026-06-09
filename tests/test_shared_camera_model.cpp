#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include "../include/siyi/camera/core/ack_policy.hpp"
#include "../include/siyi/camera/models/shared_camera_model.hpp"

namespace SIYI
{
    class TestBaseCamera final : public SharedCameraModel
    {
    public:
        explicit TestBaseCamera(
            std::shared_ptr<TelecommandSession> session = std::make_shared<TelecommandSession>())
            : SharedCameraModel(-90, 25, -270, 270, -360, 360, session){
        }
    };

    TEST_SUITE("SIYI Base Camera")
    {
        TEST_CASE("TestBaseCamera constructor initializes with correct limits")
        {
            TestBaseCamera camera;

            CHECK(camera.GetPitchMin() == -90);
            CHECK(camera.GetPitchMax() == 25);
            CHECK(camera.GetYawMin() == -270);
            CHECK(camera.GetYawMax() == 270);
            CHECK(camera.GetRollMin() == -360);
            CHECK(camera.GetRollMax() == 360);
        }
        TEST_CASE("Photo-record command keeps payload and uses no-ACK control byte")
        {
            TestBaseCamera camera;
            const auto frame = camera.TakePicture();

            SIYIPacket packet;
            REQUIRE(camera.DecodeFrame(frame, packet));
            CHECK(packet.ctrl == static_cast<uint8_t>(ControlFlag::NO_ACK));
            CHECK(packet.cmdId == static_cast<uint8_t>(CommandId::PHOTO_RECORD));
            REQUIRE(packet.data.size() == 1);
            CHECK(packet.data[0] == static_cast<uint8_t>(PhotoRecordFunction::TAKE_PICTURE));
        }

        TEST_CASE("BuildCustomCommand reflects needAck flag")
        {
            TestBaseCamera camera;

            const auto frameAck = camera.BuildCustomCommand(0x44, {0xAA}, true);
            const auto frameNoAck = camera.BuildCustomCommand(0x44, {0xAA}, false);

            SIYIPacket packetAck;
            SIYIPacket packetNoAck;

            REQUIRE(camera.DecodeFrame(frameAck, packetAck));
            REQUIRE(camera.DecodeFrame(frameNoAck, packetNoAck));

            CHECK(packetAck.ctrl == static_cast<uint8_t>(ControlFlag::NEED_ACK));
            CHECK(packetNoAck.ctrl == static_cast<uint8_t>(ControlFlag::NO_ACK));
            CHECK(packetAck.cmdId == 0x44);
            CHECK(packetNoAck.cmdId == 0x44);
            CHECK(packetAck.data == std::vector<uint8_t>({0xAA}));
            CHECK(packetNoAck.data == std::vector<uint8_t>({0xAA}));
        }

        TEST_CASE("Sequence increments on each encode")
        {
            TestBaseCamera camera;

            const auto firstFrame = camera.TakePicture();
            const auto secondFrame = camera.StartStopRecording();
            const auto thirdFrame = camera.BuildCustomCommand(0xAA, {0x01}, true);

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
            TestBaseCamera camera;

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
            TestBaseCamera firstCamera(sharedSession);
            TestBaseCamera secondCamera(sharedSession);

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
            TestBaseCamera camera;

            auto pictureDirect = camera.ControlPhotoRecord(PhotoRecordFunction::TAKE_PICTURE);
            auto pictureConv = camera.TakePicture();

            auto recordDirect = camera.ControlPhotoRecord(PhotoRecordFunction::START_STOP_RECORDING);
            auto recordConv = camera.StartStopRecording();

            auto hdrDirect = camera.ControlPhotoRecord(PhotoRecordFunction::TOGGLE_HDR);
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

            CHECK(pictureDirectPacket.data == pictureConvPacket.data);
            CHECK(recordDirectPacket.data == recordConvPacket.data);
            CHECK(hdrDirectPacket.data == hdrConvPacket.data);
        }

        TEST_CASE("Base commands encode expected IDs and payloads")
        {
            TestBaseCamera camera;

            const auto fwFrame = camera.AcquireFirmwareVersion();
            const auto hwFrame = camera.AcquireHardwareId();
            const auto centerFrame = camera.Center();
            const auto feedbackFrame = camera.AcquireFunctionFeedbackInfo();
            const auto softRestartFrame = camera.SoftRestart(true, false);

            SIYIPacket fwPacket;
            SIYIPacket hwPacket;
            SIYIPacket centerPacket;
            SIYIPacket feedbackPacket;
            SIYIPacket softRestartPacket;

            REQUIRE(camera.DecodeFrame(fwFrame, fwPacket));
            REQUIRE(camera.DecodeFrame(hwFrame, hwPacket));
            REQUIRE(camera.DecodeFrame(centerFrame, centerPacket));
            REQUIRE(camera.DecodeFrame(feedbackFrame, feedbackPacket));
            REQUIRE(camera.DecodeFrame(softRestartFrame, softRestartPacket));

            CHECK(fwPacket.cmdId == static_cast<uint8_t>(CommandId::ACQUIRE_FW_VER));
            CHECK(hwPacket.cmdId == static_cast<uint8_t>(CommandId::ACQUIRE_HW_ID));
            CHECK(centerPacket.data == std::vector<uint8_t>({0x01}));
            CHECK(feedbackPacket.cmdId == static_cast<uint8_t>(CommandId::FUNC_FEEDBACK_INFO));
            CHECK(softRestartPacket.data == std::vector<uint8_t>({1, 0}));
        }

        TEST_CASE("SetUtcTime encodes little-endian uint64 payload")
        {
            TestBaseCamera camera;

            constexpr uint64_t kUtcUs = 0x0102030405060708ULL;
            const auto frame = camera.SetUtcTime(kUtcUs);

            SIYIPacket packet;
            REQUIRE(camera.DecodeFrame(frame, packet));

            CHECK(packet.cmdId == static_cast<uint8_t>(CommandId::SET_UTC_TIME));
            CHECK(packet.data == std::vector<uint8_t>({0x08, 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01}));
        }

        TEST_CASE("SetGimbalAngle int overload encodes deci-degree payload")
        {
            TestBaseCamera camera;

            const auto frame = camera.SetGimbalAngle(static_cast<int16_t>(10), static_cast<int16_t>(-20));

            SIYIPacket packet;
            REQUIRE(camera.DecodeFrame(frame, packet));

            CHECK(packet.cmdId == static_cast<uint8_t>(CommandId::SET_GIMBAL_ANGLE));
            CHECK(packet.data == std::vector<uint8_t>({0x64, 0x00, 0x38, 0xFF}));
        }

        TEST_CASE("SetGimbalAngle float overload converts degrees to deci-degrees")
        {
            TestBaseCamera camera;

            const auto frame = camera.SetGimbalAngle(15.5F, -2.3F);

            SIYIPacket packet;
            REQUIRE(camera.DecodeFrame(frame, packet));

            CHECK(packet.data == std::vector<uint8_t>({0x9B, 0x00, 0xE9, 0xFF}));
        }

        TEST_CASE("SetGimbalAngle float overload rejects out-of-range values")
        {
            TestBaseCamera camera;

            CHECK_THROWS_AS(camera.SetGimbalAngle(300.0F, 0.0F), std::out_of_range);
            CHECK_THROWS_AS(camera.SetGimbalAngle(0.0F, -100.0F), std::out_of_range);
        }

        TEST_CASE("SetGimbalAngle accepts exact boundary values")
        {
            TestBaseCamera camera;

            const auto minFrame = camera.SetGimbalAngle(static_cast<int16_t>(-270), static_cast<int16_t>(-90));
            const auto maxFrame = camera.SetGimbalAngle(static_cast<int16_t>(270), static_cast<int16_t>(25));

            SIYIPacket minPacket;
            SIYIPacket maxPacket;
            REQUIRE(camera.DecodeFrame(minFrame, minPacket));
            REQUIRE(camera.DecodeFrame(maxFrame, maxPacket));

            CHECK(minPacket.data == std::vector<uint8_t>({0x74, 0xF5, 0x7C, 0xFC}));
            CHECK(maxPacket.data == std::vector<uint8_t>({0x8C, 0x0A, 0xFA, 0x00}));
        }

        TEST_CASE("SetGimbalAngle int overload rejects out-of-range values")
        {
            TestBaseCamera camera;

            CHECK_THROWS_AS(camera.SetGimbalAngle(static_cast<int16_t>(271), static_cast<int16_t>(0)), std::out_of_range);
            CHECK_THROWS_AS(camera.SetGimbalAngle(static_cast<int16_t>(0), static_cast<int16_t>(26)), std::out_of_range);
        }

        TEST_CASE("Decode typed gimbal attitude telemetry")
        {
            TestBaseCamera camera;

            SIYIPacket packet;
            packet.ctrl = MakeDeviceAckControlByte();
            packet.cmdId = static_cast<uint8_t>(CommandId::ACQUIRE_GIMBAL_ATT);
            packet.data = {0x7B, 0x00, 0xCE, 0xFF, 0x00, 0x00, 0x0F, 0x00, 0xFB, 0xFF, 0x2C, 0x01};
            packet.dataLen = static_cast<uint16_t>(packet.data.size());

            TM::TelemetryMessage message;
            std::string error;
            REQUIRE(camera.DecodeTelemetryPacket(packet, message, &error));

            const auto* att = std::get_if<TM::GimbalAttitude>(&message);
            REQUIRE(att != nullptr);
            CHECK(att->yaw == 123);
            CHECK(att->pitch == -50);
            CHECK(att->rollVelocity == 300);
        }

        TEST_CASE("Decode typed function-feedback telemetry")
        {
            TestBaseCamera camera;

            SIYIPacket packet;
            packet.ctrl = MakeDeviceAckControlByte();
            packet.cmdId = static_cast<uint8_t>(CommandId::FUNC_FEEDBACK_INFO);
            packet.data = {static_cast<uint8_t>(TM::FeedbackInfoType::HDR_ON)};
            packet.dataLen = static_cast<uint16_t>(packet.data.size());

            TM::TelemetryMessage message;
            std::string error;
            REQUIRE(camera.DecodeTelemetryPacket(packet, message, &error));

            const auto* info = std::get_if<TM::FuncFeedbackInfo>(&message);
            REQUIRE(info != nullptr);
            CHECK(info->AsFuncFeedbackInfoType() == TM::FeedbackInfoType::HDR_ON);
        }

        TEST_CASE("Decode typed firmware-version telemetry")
        {
            TestBaseCamera camera;

            SIYIPacket packet;
            packet.ctrl = MakeDeviceAckControlByte();
            packet.cmdId = static_cast<uint8_t>(CommandId::ACQUIRE_FW_VER);
            packet.data = {0x04, 0x03, 0x02, 0x01, 0x08, 0x07, 0x06, 0x05, 0x0C, 0x0B, 0x0A, 0x09};
            packet.dataLen = static_cast<uint16_t>(packet.data.size());

            TM::TelemetryMessage message;
            std::string error;
            REQUIRE(camera.DecodeTelemetryPacket(packet, message, &error));

            const auto* fw = std::get_if<TM::FirmwareVersion>(&message);
            REQUIRE(fw != nullptr);
            CHECK(fw->cameraFirmwareVersion == 0x01020304U);
            CHECK(fw->gimbalFirmwareVersion == 0x05060708U);
            CHECK(fw->zoomFirmwareVersion == 0x090A0B0CU);
        }

        TEST_CASE("Decode one-byte payload as generic command status ack")
        {
            TestBaseCamera camera;

            SIYIPacket packet;
            packet.ctrl = MakeDeviceAckControlByte();
            packet.cmdId = static_cast<uint8_t>(CommandId::CENTER);
            packet.data = {0x01};
            packet.dataLen = static_cast<uint16_t>(packet.data.size());

            TM::TelemetryMessage message;
            std::string error;
            REQUIRE(camera.DecodeTelemetryPacket(packet, message, &error));

            const auto* ack = std::get_if<TM::CommandStatusAck>(&message);
            REQUIRE(ack != nullptr);
            CHECK(ack->cmdId == static_cast<uint8_t>(CommandId::CENTER));
            CHECK(ack->status == 0x01);
            CHECK(ack->IsSuccess());
        }

        TEST_CASE("Reject invalid function-feedback payload length")
        {
            TestBaseCamera camera;

            SIYIPacket packet;
            packet.ctrl = MakeDeviceAckControlByte();
            packet.cmdId = static_cast<uint8_t>(CommandId::FUNC_FEEDBACK_INFO);
            packet.data = {0x00, 0x01};
            packet.dataLen = static_cast<uint16_t>(packet.data.size());

            TM::TelemetryMessage message;
            std::string error;
            CHECK_FALSE(camera.DecodeTelemetryPacket(packet, message, &error));
            CHECK(error.find("1 byte") != std::string::npos);
        }

        TEST_CASE("ACK policy maps photo-record ACK from 0x0C to 0x0B")
        {
            const uint8_t requestCmdId = static_cast<uint8_t>(CommandId::PHOTO_RECORD);
            const uint8_t feedbackCmdId = static_cast<uint8_t>(CommandId::FUNC_FEEDBACK_INFO);

            CHECK(AckPolicy::ExpectedAckCmdIdForRequest(requestCmdId) == feedbackCmdId);
            CHECK(AckPolicy::IsExpectedAckCmdIdForRequest(feedbackCmdId, requestCmdId));
            CHECK_FALSE(AckPolicy::IsExpectedAckCmdIdForRequest(static_cast<uint8_t>(CommandId::ACQUIRE_FW_VER), requestCmdId));
        }

        TEST_CASE("AcquireCameraCodecSpecs encodes correct command ID and stream type")
        {
            TestBaseCamera camera;

            const auto recordingFrame = camera.AcquireCameraCodecSpecs(StreamType::RECORDING_STREAM);
            const auto mainFrame = camera.AcquireCameraCodecSpecs(StreamType::MAIN_STREAM);
            const auto subFrame = camera.AcquireCameraCodecSpecs(StreamType::SUB_STREAM);

            SIYIPacket recordingPacket;
            SIYIPacket mainPacket;
            SIYIPacket subPacket;

            REQUIRE(camera.DecodeFrame(recordingFrame, recordingPacket));
            REQUIRE(camera.DecodeFrame(mainFrame, mainPacket));
            REQUIRE(camera.DecodeFrame(subFrame, subPacket));

            CHECK(recordingPacket.cmdId == static_cast<uint8_t>(CommandId::ACQUIRE_CODEC_SPECS));
            CHECK(recordingPacket.ctrl == static_cast<uint8_t>(ControlFlag::NEED_ACK));
            CHECK(recordingPacket.data == std::vector<uint8_t>({0x00}));

            CHECK(mainPacket.cmdId == static_cast<uint8_t>(CommandId::ACQUIRE_CODEC_SPECS));
            CHECK(mainPacket.data == std::vector<uint8_t>({0x01}));

            CHECK(subPacket.cmdId == static_cast<uint8_t>(CommandId::ACQUIRE_CODEC_SPECS));
            CHECK(subPacket.data == std::vector<uint8_t>({0x02}));
        }

        TEST_CASE("SendCameraCodecSpecs encodes 9-byte little-endian payload")
        {
            TestBaseCamera camera;

            // 1920 = 0x0780, 1080 = 0x0438, 4000 Kbps = 0x0FA0
            const auto frame = camera.SendCameraCodecSpecs(
                StreamType::MAIN_STREAM,
                VideoEncType::H264,
                1920,
                1080,
                4000);

            SIYIPacket packet;
            REQUIRE(camera.DecodeFrame(frame, packet));

            CHECK(packet.cmdId == static_cast<uint8_t>(CommandId::SEND_CODEC_SPECS));
            CHECK(packet.ctrl == static_cast<uint8_t>(ControlFlag::NEED_ACK));
            REQUIRE(packet.data.size() == 9);
            CHECK(packet.data[0] == static_cast<uint8_t>(StreamType::MAIN_STREAM));
            CHECK(packet.data[1] == static_cast<uint8_t>(VideoEncType::H264));
            CHECK(packet.data[2] == 0x80); // 1920 low byte
            CHECK(packet.data[3] == 0x07); // 1920 high byte
            CHECK(packet.data[4] == 0x38); // 1080 low byte
            CHECK(packet.data[5] == 0x04); // 1080 high byte
            CHECK(packet.data[6] == 0xA0); // 4000 low byte
            CHECK(packet.data[7] == 0x0F); // 4000 high byte
            CHECK(packet.data[8] == 0x00); // reserved
        }

        TEST_CASE("Decode CameraCodecSpecs ACK from ACQUIRE_CODEC_SPECS response")
        {
            TestBaseCamera camera;

            // 1280 = 0x0500, 720 = 0x02D0, 2048 Kbps = 0x0800, 30 fps
            SIYIPacket packet;
            packet.ctrl = MakeDeviceAckControlByte();
            packet.cmdId = static_cast<uint8_t>(CommandId::ACQUIRE_CODEC_SPECS);
            packet.data = {
                0x01,       // MAIN_STREAM
                0x02,       // H265
                0x00, 0x05, // width  1280 LE
                0xD0, 0x02, // height  720 LE
                0x00, 0x08, // bitrate 2048 Kbps LE
                0x1E        // 30 fps
            };
            packet.dataLen = static_cast<uint16_t>(packet.data.size());

            TM::TelemetryMessage message;
            std::string error;
            REQUIRE(camera.DecodeTelemetryPacket(packet, message, &error));

            const auto* specs = std::get_if<TM::CameraCodecSpecs>(&message);
            REQUIRE(specs != nullptr);
            CHECK(specs->AsStreamType() == StreamType::MAIN_STREAM);
            CHECK(specs->AsVideoEncType() == VideoEncType::H265);
            CHECK(specs->resolutionWidth == 1280);
            CHECK(specs->resolutionHeight == 720);
            CHECK(specs->videoBitrateKbps == 2048);
            CHECK(specs->videoFrameRate == 30);

            CHECK(camera.HasCurrentResolution());
            CHECK(camera.GetCurrentResolutionWidth() == 1280);
            CHECK(camera.GetCurrentResolutionHeight() == 720);
        }

        TEST_CASE("Decode SendCodecSpecsAck from SEND_CODEC_SPECS response")
        {
            TestBaseCamera camera;

            // Stage desired codec resolution; successful ACK should commit these values.
            const auto request = camera.SendCameraCodecSpecs(
                StreamType::RECORDING_STREAM,
                VideoEncType::H264,
                1920,
                1080,
                3500);
            SIYIPacket requestPacket;
            REQUIRE(camera.DecodeFrame(request, requestPacket));

            SIYIPacket successPacket;
            successPacket.ctrl = MakeDeviceAckControlByte();
            successPacket.cmdId = static_cast<uint8_t>(CommandId::SEND_CODEC_SPECS);
            successPacket.data = {0x00, 0x01}; // RECORDING_STREAM, success
            successPacket.dataLen = static_cast<uint16_t>(successPacket.data.size());

            SIYIPacket failPacket;
            failPacket.ctrl = MakeDeviceAckControlByte();
            failPacket.cmdId = static_cast<uint8_t>(CommandId::SEND_CODEC_SPECS);
            failPacket.data = {0x01, 0x00}; // MAIN_STREAM, failure
            failPacket.dataLen = static_cast<uint16_t>(failPacket.data.size());

            TM::TelemetryMessage successMessage;
            TM::TelemetryMessage failMessage;
            std::string error;

            REQUIRE(camera.DecodeTelemetryPacket(successPacket, successMessage, &error));
            REQUIRE(camera.DecodeTelemetryPacket(failPacket, failMessage, &error));

            const auto* successAck = std::get_if<TM::SendCodecSpecsAck>(&successMessage);
            REQUIRE(successAck != nullptr);
            CHECK(successAck->AsStreamType() == StreamType::RECORDING_STREAM);
            CHECK(successAck->IsSuccess());
            CHECK(camera.HasCurrentResolution());
            CHECK(camera.GetCurrentResolutionWidth() == 1920);
            CHECK(camera.GetCurrentResolutionHeight() == 1080);

            const auto* failAck = std::get_if<TM::SendCodecSpecsAck>(&failMessage);
            REQUIRE(failAck != nullptr);
            CHECK(failAck->AsStreamType() == StreamType::MAIN_STREAM);
            CHECK_FALSE(failAck->IsSuccess());
            CHECK(camera.GetCurrentResolutionWidth() == 1920);
            CHECK(camera.GetCurrentResolutionHeight() == 1080);
        }

        TEST_CASE("Reject ACQUIRE_CODEC_SPECS ACK with wrong payload length")
        {
            TestBaseCamera camera;

            SIYIPacket packet;
            packet.ctrl = MakeDeviceAckControlByte();
            packet.cmdId = static_cast<uint8_t>(CommandId::ACQUIRE_CODEC_SPECS);
            packet.data = {0x01, 0x02, 0x03}; // too short
            packet.dataLen = static_cast<uint16_t>(packet.data.size());

            TM::TelemetryMessage message;
            std::string error;
            CHECK_FALSE(camera.DecodeTelemetryPacket(packet, message, &error));
            CHECK(error.find("9 bytes") != std::string::npos);
        }

        TEST_CASE("Reject SEND_CODEC_SPECS ACK with wrong payload length")
        {
            TestBaseCamera camera;

            SIYIPacket packet;
            packet.ctrl = MakeDeviceAckControlByte();
            packet.cmdId = static_cast<uint8_t>(CommandId::SEND_CODEC_SPECS);
            packet.data = {0x01}; // too short
            packet.dataLen = static_cast<uint16_t>(packet.data.size());

            TM::TelemetryMessage message;
            std::string error;
            CHECK_FALSE(camera.DecodeTelemetryPacket(packet, message, &error));
            CHECK(error.find("2 bytes") != std::string::npos);
        }

        TEST_CASE("AcquireWorkingMode encodes correct command ID and payload")
        {
            TestBaseCamera camera;

            const auto frame = camera.AcquireGimbalWorkingMode();

            SIYIPacket packet;
            REQUIRE(camera.DecodeFrame(frame, packet));

            CHECK(packet.cmdId == static_cast<uint8_t>(CommandId::ACQUIRE_WORKING_MODE));
            CHECK(packet.ctrl == static_cast<uint8_t>(ControlFlag::NEED_ACK));
            CHECK(packet.data.empty());
        }

        TEST_CASE("Decode AcquireWorkingMode ACK with correct payload")
        {
            TestBaseCamera camera;

            SIYIPacket packet;
            packet.ctrl = MakeDeviceAckControlByte();
            packet.cmdId = static_cast<uint8_t>(CommandId::ACQUIRE_WORKING_MODE);
            packet.data = {static_cast<uint8_t>(TM::GimbalWorkingModeEnum::FOLLOW_MODE)}; // e.g. "Follow" mode
            packet.dataLen = static_cast<uint16_t>(packet.data.size());

            TM::TelemetryMessage message;
            std::string error;
            REQUIRE(camera.DecodeTelemetryPacket(packet, message, &error));

            const auto* mode = std::get_if<TM::GimbalWorkingMode>(&message);
            REQUIRE(mode != nullptr);
            CHECK(mode->AsGimbalWorkingMode() == TM::GimbalWorkingModeEnum::FOLLOW_MODE);
        }
    }
} // namespace SIYI
