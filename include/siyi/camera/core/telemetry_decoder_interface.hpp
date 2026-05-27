#ifndef SIYI_TELEMETRY_DECODER_INTERFACE_HPP
#define SIYI_TELEMETRY_DECODER_INTERFACE_HPP

#include "../protocol/tm_parameters.hpp"

#include <cstdint>
#include <string>
#include <vector>

namespace SIYI
{
    struct SIYIPacket;

    // Backward-compatible aliases so higher-level APIs can refer to codec enums
    // from SIYI namespace even though they are generated under SIYI::TM.
    using StreamType = TM::StreamType;
    using VideoEncType = TM::VideoEncType;

    class ITelemetryDecoder
    {
    public:
        virtual ~ITelemetryDecoder() = default;

        virtual bool DecodeFrame(
            const std::vector<uint8_t>& frame,
            SIYIPacket& outPacket,
            std::string* error = nullptr) const = 0;

        virtual bool DecodeTelemetryPacket(
            const SIYIPacket& packet,
            TM::TelemetryMessage& outMessage,
            std::string* error = nullptr) const = 0;

        virtual bool DecodeTelemetryFrame(
            const std::vector<uint8_t>& frame,
            TM::TelemetryMessage& outMessage,
            std::string* error = nullptr) const = 0;
    };
} // namespace SIYI

#endif // SIYI_TELEMETRY_DECODER_INTERFACE_HPP
