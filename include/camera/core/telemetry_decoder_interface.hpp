#ifndef SIYI_TELEMETRY_DECODER_INTERFACE_HPP
#define SIYI_TELEMETRY_DECODER_INTERFACE_HPP

#include "../protocol/tm_parameters.hpp"

#include <cstdint>
#include <string>
#include <vector>

namespace SIYI
{
    struct SIYIPacket;

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
