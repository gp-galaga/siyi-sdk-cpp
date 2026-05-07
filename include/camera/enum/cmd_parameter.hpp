#ifndef SIYI_CAMERA_CMD_PARAMETER_HPP
#define SIYI_CAMERA_CMD_PARAMETER_HPP

#include <cstdint>

namespace SIYI
{
    // Control byte values (SIYI wire protocol):
    //   0x00  host -> device, no ACK requested
    //   0x01  host -> device, ACK requested
    //   0x02  device -> host, ACK response

    enum class ControlFlag : uint8_t
    {
        NO_ACK   = 0x00,
        NEED_ACK = 0x01,
        ACK      = 0x02
    };

    // Returns the control byte for a host-originated request that expects an ACK.
    inline uint8_t MakeHostRequestControlByte() noexcept
    {
        return static_cast<uint8_t>(ControlFlag::NEED_ACK);
    }

    // Returns the control byte for a device-originated ACK response.
    inline uint8_t MakeDeviceAckControlByte() noexcept
    {
        return static_cast<uint8_t>(ControlFlag::ACK);
    }

    enum class CommandId : uint8_t
    {
        // To be tested
        ACQUIRE_FW_VER = 0x01,
        ACQUIRE_HW_ID = 0x02,
        AUTO_FOCUS = 0x04,
        CENTER = 0x08,
        ABSOLUTE_ZOOM = 0x0f,
        SET_UTC_TIME = 0x30,
        SOFT_RESTART = 0x80,


        ACQUIRE_GIMBAL_CONFIGURATION = 0x0a,
        FUNC_FEEDBACK_INFO = 0x0b,
        ACQUIRE_GIMBAL_ATT = 0x0d,
        ZOOM = 0x05,
        MANUAL_FOCUS = 0x06,
        ROTATION = 0x07,
        PHOTO_RECORD = 0x0C
    };

    enum class PhotoRecordFunction : uint8_t
    {
        TAKE_PICTURE = 0,
        TOGGLE_HDR = 1,
        START_STOP_RECORDING = 2,
        MOTION_LOCK_MODE = 3,
        MOTION_FOLLOW_MODE = 4,
        MOTION_FPV_MODE = 5,
        VIDEO_OUTPUT_HDMI = 6,
        VIDEO_OUTPUT_CVBS = 7,
        VIDEO_OUTPUT_OFF = 8
    };

    enum class ManualZoomDirection : int8_t
    {
        STOP = 0,
        ZOOM_IN = 1,
        ZOOM_OUT = -1
    };
}

#endif