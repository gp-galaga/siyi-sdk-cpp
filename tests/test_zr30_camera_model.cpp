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
            CHECK(camera.GetRollMin() == -45);
            CHECK(camera.GetRollMax() == 45);
            CHECK(camera.GetZoomOpticalMin() == 0);
            CHECK(camera.GetZoomOpticalMax() == 30);
            CHECK(camera.GetZoomMin() == 0);
            CHECK(camera.GetZoomMax() == 180);

            const auto& technicalSpecs = camera.GetTechnicalSpecs();
            CHECK(technicalSpecs.angularVibrationRangeDeg == doctest::Approx(0.01F));
            CHECK(technicalSpecs.sensorDescription == "1/2.7 Inch Sony CMOS");
            CHECK(technicalSpecs.effectiveResolutionMegaPixels == doctest::Approx(8.0F));
            REQUIRE(technicalSpecs.supportedVideoRecordingResolutions.size() == 4);
            CHECK(technicalSpecs.supportedVideoRecordingResolutions[0].resolution.width == 3840);
            CHECK(technicalSpecs.supportedVideoRecordingResolutions[0].resolution.height == 2160);
            CHECK(technicalSpecs.supportedVideoRecordingResolutions[0].fps == 25);
            REQUIRE(technicalSpecs.supportedStillPhotoResolutions.size() == 1);
            CHECK(technicalSpecs.supportedStillPhotoResolutions[0].width == 3840);
            CHECK(technicalSpecs.supportedStillPhotoResolutions[0].height == 2160);
            CHECK(technicalSpecs.imageFormat == "JPG");
            CHECK(technicalSpecs.videoFileFormat == "MP4");

            const auto& opticalTechnicalSpecs = camera.GetOpticalTechnicalSpecs();
            CHECK(opticalTechnicalSpecs.opticalZoomMaxX == doctest::Approx(30.0F));
            CHECK(opticalTechnicalSpecs.hybridZoomMaxX == doctest::Approx(180.0F));
            CHECK(opticalTechnicalSpecs.focalLengthMinMm == doctest::Approx(4.5F));
            CHECK(opticalTechnicalSpecs.focalLengthMaxMm == doctest::Approx(148.4F));
            CHECK(opticalTechnicalSpecs.apertureMinF == doctest::Approx(1.3F));
            CHECK(opticalTechnicalSpecs.apertureMaxF == doctest::Approx(2.8F));
            CHECK(opticalTechnicalSpecs.fovDiagonalMinZoomDeg == doctest::Approx(65.4F));
            CHECK(opticalTechnicalSpecs.fovHorizontalMinZoomDeg == doctest::Approx(58.1F));
            CHECK(opticalTechnicalSpecs.fovDiagonalMaxOpticalZoomDeg == doctest::Approx(2.5F));
            CHECK(opticalTechnicalSpecs.fovHorizontalMaxOpticalZoomDeg == doctest::Approx(2.1F));
        }

        TEST_CASE("ZR30 inherits optical zoom command implementations")
        {
            ZR30 camera;

            const auto autoFocusFrame = camera.AutoFocus(100, 200);
            const auto manualFocusFrame = camera.SetManualFocus(ManualFocusDirection::LONG_SHOT);
            const auto absZoomFrame = camera.SetAbsoluteZoom(static_cast<float>(4.5));

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