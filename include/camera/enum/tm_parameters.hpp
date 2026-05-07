#ifndef __SIYI_CAMERA_TM_HPP__
#define __SIYI_CAMERA_TM_HPP__

#include <cstdint>
#include <variant>

namespace SIYI
{
	namespace TM
	{
		struct GimbalAttitudeTM
		{
			int16_t yaw;
			int16_t pitch;
			int16_t roll;
			int16_t yawVelocity;
			int16_t pitchVelocity;
			int16_t rollVelocity;

			double YawDeg() const { return static_cast<double>(yaw) / 10.0; }
			double PitchDeg() const { return static_cast<double>(pitch) / 10.0; }
			double RollDeg() const { return static_cast<double>(roll) / 10.0; }

			double YawVelocityDegPerSec() const { return static_cast<double>(yawVelocity) / 10.0; }
			double PitchVelocityDegPerSec() const { return static_cast<double>(pitchVelocity) / 10.0; }
			double RollVelocityDegPerSec() const { return static_cast<double>(rollVelocity) / 10.0; }
		};

		struct GimbalConfigurationTM
		{
			uint8_t reserved0;
			uint8_t hdrStatus;             	// 0: OFF, 1: ON
			uint8_t reserved1;
			uint8_t recordStatus;          	// 0: OFF, 1: ON, 2: TF empty, 3: TF data loss
			uint8_t gimbalMotionMode;      	// 1: Follow, 2: FPV
			uint8_t gimbalMountingMethod;  	// 0: Reserved, 1: Normal, 2: Upside Down
			uint8_t video_hdmi_or_cvbs; 	// 0: HDMI output ON, 1: CVBS output ON
		};

		enum class COMMAND_ACKNOLEDGEMENT : uint8_t
		{
			ACK_SUCCESS = 0,
			ACK_FAIL = 1,
		};

		enum class GimbalModel : uint8_t
		{
			UNKNOWN = 0,
			ZR10 = 0x6B,
			A8_mini = 0x73,
			A2_mini = 0x75,
			ZR30 = 0x78,
			ZT6 = 0x82,
			ZT30 = 0x7A,
		};

		struct GimbalHardwareId{
			uint8_t gimbalModel;			// 0: Unknown, 1: ZR30
		};
		using TelemetryMessage = std::variant<GimbalAttitudeTM, GimbalConfigurationTM, GimbalHardwareId>;
	};
}

#endif // __SIYI_CAMERA_TM_HPP__