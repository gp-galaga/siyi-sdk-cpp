#include "../../include/siyi/camera/models/zr30_camera_model.hpp"
#include <iostream>

namespace SIYI
{
    namespace
    {
        CameraTechnicalSpecs BuildZr30TechnicalSpecs()
        {
            CameraTechnicalSpecs specs;
            specs.angularVibrationRangeDeg = 0.01F;
            specs.sensorDescription = "1/2.7 Inch Sony CMOS";
            specs.effectiveResolutionMegaPixels = 8.0F;
            specs.supportedVideoRecordingResolutions = {
                {{3840, 2160}, 25},
                {{2560, 1440}, 30},
                {{1920, 1080}, 30},
                {{1280, 720}, 30}};
            specs.supportedStillPhotoResolutions = {{3840, 2160}};
            specs.imageFormat = "JPG";
            specs.videoFileFormat = "MP4";
            return specs;
        }

        OpticalZoomTechnicalSpecs BuildZr30OpticalTechnicalSpecs()
        {
            OpticalZoomTechnicalSpecs specs;
            specs.opticalZoomMaxX = 30.0F;
            specs.hybridZoomMaxX = 180.0F;
            specs.focalLengthMinMm = 4.5F;
            specs.focalLengthMaxMm = 148.4F;
            specs.apertureMinF = 1.3F;
            specs.apertureMaxF = 2.8F;
            specs.fovDiagonalMinZoomDeg = 65.4F;
            specs.fovHorizontalMinZoomDeg = 58.1F;
            specs.fovDiagonalMaxOpticalZoomDeg = 2.5F;
            specs.fovHorizontalMaxOpticalZoomDeg = 2.1F;
            return specs;
        }
    } // namespace

    ZR30::ZR30(std::shared_ptr<TelecommandSession> session)
        : OpticalZoomCameraModel(
              -90,
              25,
              -270,
              270,
              -45,
              45,
              0,
              180,
              0,
              30,
              session,
              BuildZr30TechnicalSpecs(),
              BuildZr30OpticalTechnicalSpecs()){
            std::cout << "ZR30 camera model initialized with specific limits and capabilities.\n";
            SetCameraName("ZR30");
        }
} // namespace SIYI