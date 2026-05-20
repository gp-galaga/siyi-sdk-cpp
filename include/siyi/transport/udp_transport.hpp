#ifndef SIYI_UDP_TRANSPORT_HPP
#define SIYI_UDP_TRANSPORT_HPP

#include "itransport.hpp"

#include <netinet/in.h>

namespace SIYI
{
    class UdpTransport final : public ITransport
    {
    public:
        UdpTransport() = default;
        ~UdpTransport() override;

        bool Open(
            const std::string& ipAddress,
            uint16_t remotePort,
            uint16_t bindPort,
            std::string* error = nullptr) override;

        bool Send(
            const std::vector<uint8_t>& frame,
            std::string* error = nullptr) override;

        bool Receive(
            std::vector<uint8_t>& frame,
            int timeoutMs,
            std::string* error = nullptr) override;

        void Close() noexcept override;

    private:
        int socketFd_ {-1};
        sockaddr_in remoteAddress_ {};
    };
} // namespace SIYI

#endif // SIYI_UDP_TRANSPORT_HPP
