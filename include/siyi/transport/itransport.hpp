#ifndef SIYI_ITRANSPORT_HPP
#define SIYI_ITRANSPORT_HPP

#include <cstdint>
#include <string>
#include <vector>

namespace SIYI
{
    class ITransport
    {
    public:
        virtual ~ITransport() = default;

        virtual bool Open(
            const std::string& ipAddress,
            uint16_t remotePort,
            uint16_t bindPort,
            std::string* error = nullptr) = 0;

        virtual bool Send(
            const std::vector<uint8_t>& frame,
            std::string* error = nullptr) = 0;

        virtual bool Receive(
            std::vector<uint8_t>& frame,
            int timeoutMs,
            std::string* error = nullptr) = 0;

        virtual void Close() noexcept = 0;
    };
} // namespace SIYI

#endif // SIYI_ITRANSPORT_HPP
