#include "../../include/siyi/transport/udp_transport.hpp"

#include <arpa/inet.h>
#include <cerrno>
#include <cstring>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>

namespace SIYI
{
    UdpTransport::~UdpTransport()
    {
        Close();
    }

    bool UdpTransport::Open(
        const std::string& ipAddress,
        uint16_t remotePort,
        uint16_t bindPort,
        std::string* error)
    {
        Close();

        socketFd_ = ::socket(AF_INET, SOCK_DGRAM, 0);
        if (socketFd_ < 0)
        {
            if (error != nullptr)
            {
                *error = std::string("socket() failed: ") + std::strerror(errno);
            }
            return false;
        }

        if (bindPort != 0)
        {
            sockaddr_in localAddress {};
            localAddress.sin_family = AF_INET;
            localAddress.sin_addr.s_addr = htonl(INADDR_ANY);
            localAddress.sin_port = htons(bindPort);
            if (::bind(socketFd_, reinterpret_cast<const sockaddr*>(&localAddress), sizeof(localAddress)) != 0)
            {
                if (error != nullptr)
                {
                    *error = std::string("bind() failed: ") + std::strerror(errno);
                }
                Close();
                return false;
            }
        }

        remoteAddress_ = {};
        remoteAddress_.sin_family = AF_INET;
        remoteAddress_.sin_port = htons(remotePort);
        if (::inet_pton(AF_INET, ipAddress.c_str(), &remoteAddress_.sin_addr) != 1)
        {
            if (error != nullptr)
            {
                *error = "invalid IPv4 address: " + ipAddress;
            }
            Close();
            return false;
        }

        return true;
    }

    bool UdpTransport::Send(const std::vector<uint8_t>& frame, std::string* error)
    {
        const ssize_t sent = ::sendto(
            socketFd_,
            frame.data(),
            frame.size(),
            0,
            reinterpret_cast<const sockaddr*>(&remoteAddress_),
            sizeof(remoteAddress_));
        if (sent < 0)
        {
            if (error != nullptr)
            {
                *error = std::string("sendto() failed: ") + std::strerror(errno);
            }
            return false;
        }
        return true;
    }

    bool UdpTransport::Receive(std::vector<uint8_t>& frame, int timeoutMs, std::string* error)
    {
        fd_set readSet;
        FD_ZERO(&readSet);
        FD_SET(socketFd_, &readSet);

        timeval timeout {};
        timeout.tv_sec = timeoutMs / 1000;
        timeout.tv_usec = (timeoutMs % 1000) * 1000;

        const int ready = ::select(socketFd_ + 1, &readSet, nullptr, nullptr, &timeout);
        if (ready <= 0)
        {
            if (error != nullptr)
            {
                *error = (ready == 0) ? "recv timed out" : std::string("select() failed: ") + std::strerror(errno);
            }
            return false;
        }

        std::vector<uint8_t> response(2048);
        sockaddr_in sourceAddress {};
        socklen_t sourceLength = sizeof(sourceAddress);
        const ssize_t received = ::recvfrom(
            socketFd_,
            response.data(),
            response.size(),
            0,
            reinterpret_cast<sockaddr*>(&sourceAddress),
            &sourceLength);
        if (received < 0)
        {
            if (error != nullptr)
            {
                *error = std::string("recvfrom() failed: ") + std::strerror(errno);
            }
            return false;
        }

        response.resize(static_cast<size_t>(received));
        frame = std::move(response);
        return true;
    }

    void UdpTransport::Close() noexcept
    {
        if (socketFd_ >= 0)
        {
            ::close(socketFd_);
            socketFd_ = -1;
        }
    }
} // namespace SIYI
