#include "../../include/transport/tcp_transport.hpp"

#include <algorithm>
#include <arpa/inet.h>
#include <cerrno>
#include <cstring>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>

namespace SIYI
{
    namespace
    {
        constexpr uint8_t kStxLow = 0x55;
        constexpr uint8_t kStxHigh = 0x66;
        constexpr size_t kHeaderWithoutPayload = 8;
        constexpr size_t kCrcSize = 2;
    }

    TcpTransport::~TcpTransport()
    {
        Close();
    }

    bool TcpTransport::Open(
        const std::string& ipAddress,
        uint16_t remotePort,
        uint16_t bindPort,
        std::string* error)
    {
        Close();

        socketFd_ = ::socket(AF_INET, SOCK_STREAM, 0);
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

        sockaddr_in remoteAddress {};
        remoteAddress.sin_family = AF_INET;
        remoteAddress.sin_port = htons(remotePort);
        if (::inet_pton(AF_INET, ipAddress.c_str(), &remoteAddress.sin_addr) != 1)
        {
            if (error != nullptr)
            {
                *error = "invalid IPv4 address: " + ipAddress;
            }
            Close();
            return false;
        }

        if (::connect(socketFd_, reinterpret_cast<const sockaddr*>(&remoteAddress), sizeof(remoteAddress)) != 0)
        {
            if (error != nullptr)
            {
                *error = std::string("connect() failed: ") + std::strerror(errno);
            }
            Close();
            return false;
        }

        receiveBuffer_.clear();
        return true;
    }

    bool TcpTransport::Send(const std::vector<uint8_t>& frame, std::string* error)
    {
        size_t written = 0;
        while (written < frame.size())
        {
            const ssize_t sent = ::send(socketFd_, frame.data() + written, frame.size() - written, 0);
            if (sent <= 0)
            {
                if (error != nullptr)
                {
                    *error = std::string("send() failed: ") + std::strerror(errno);
                }
                return false;
            }
            written += static_cast<size_t>(sent);
        }
        return true;
    }

    bool TcpTransport::ExtractFrameFromBuffer(std::vector<uint8_t>& frame)
    {
        if (receiveBuffer_.size() < 2)
        {
            return false;
        }

        auto stxIt = receiveBuffer_.begin();
        while (stxIt + 1 < receiveBuffer_.end())
        {
            if (*stxIt == kStxLow && *(stxIt + 1) == kStxHigh)
            {
                break;
            }
            ++stxIt;
        }

        if (stxIt != receiveBuffer_.begin())
        {
            receiveBuffer_.erase(receiveBuffer_.begin(), stxIt);
        }

        if (receiveBuffer_.size() < kHeaderWithoutPayload)
        {
            return false;
        }

        const uint16_t payloadLen = static_cast<uint16_t>(receiveBuffer_[3]) |
            static_cast<uint16_t>(receiveBuffer_[4] << 8);
        const size_t fullFrameSize = kHeaderWithoutPayload + static_cast<size_t>(payloadLen) + kCrcSize;
        if (receiveBuffer_.size() < fullFrameSize)
        {
            return false;
        }

        frame.assign(receiveBuffer_.begin(), receiveBuffer_.begin() + static_cast<std::ptrdiff_t>(fullFrameSize));
        receiveBuffer_.erase(receiveBuffer_.begin(), receiveBuffer_.begin() + static_cast<std::ptrdiff_t>(fullFrameSize));
        return true;
    }

    bool TcpTransport::Receive(std::vector<uint8_t>& frame, int timeoutMs, std::string* error)
    {
        frame.clear();
        if (ExtractFrameFromBuffer(frame))
        {
            return true;
        }

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

        std::vector<uint8_t> incoming(2048);
        const ssize_t bytesRead = ::recv(socketFd_, incoming.data(), incoming.size(), 0);
        if (bytesRead <= 0)
        {
            if (error != nullptr)
            {
                *error = (bytesRead == 0) ? "connection closed by peer" : std::string("recv() failed: ") + std::strerror(errno);
            }
            return false;
        }

        incoming.resize(static_cast<size_t>(bytesRead));
        receiveBuffer_.insert(receiveBuffer_.end(), incoming.begin(), incoming.end());

        if (!ExtractFrameFromBuffer(frame))
        {
            if (error != nullptr)
            {
                *error = "received incomplete TCP frame";
            }
            return false;
        }

        return true;
    }

    void TcpTransport::Close() noexcept
    {
        receiveBuffer_.clear();
        if (socketFd_ >= 0)
        {
            ::close(socketFd_);
            socketFd_ = -1;
        }
    }
} // namespace SIYI
