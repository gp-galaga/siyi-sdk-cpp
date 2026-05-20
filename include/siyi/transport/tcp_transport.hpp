#ifndef SIYI_TCP_TRANSPORT_HPP
#define SIYI_TCP_TRANSPORT_HPP

#include "itransport.hpp"

#include <vector>

namespace SIYI
{
    class TcpTransport final : public ITransport
    {
    public:
        TcpTransport() = default;
        ~TcpTransport() override;

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
        bool ExtractFrameFromBuffer(std::vector<uint8_t>& frame);

        int socketFd_ {-1};
        std::vector<uint8_t> receiveBuffer_ {};
    };
} // namespace SIYI

#endif // SIYI_TCP_TRANSPORT_HPP
