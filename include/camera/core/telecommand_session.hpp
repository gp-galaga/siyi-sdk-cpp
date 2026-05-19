#ifndef SIYI_TELECOMMAND_SESSION_HPP
#define SIYI_TELECOMMAND_SESSION_HPP

#include <atomic>
#include <cstdint>

namespace SIYI
{
    class TelecommandSession
    {
    public:
        uint16_t NextSeq() noexcept
        {
            return nextSeq_.fetch_add(1, std::memory_order_relaxed);
        }

        void ResetSeq(uint16_t nextSeq = 0) noexcept
        {
            nextSeq_.store(nextSeq, std::memory_order_relaxed);
        }

        uint16_t PeekNextSeq() const noexcept
        {
            return nextSeq_.load(std::memory_order_relaxed);
        }

    private:
        std::atomic<uint16_t> nextSeq_ {0};
    };
} // namespace SIYI

#endif // SIYI_TELECOMMAND_SESSION_HPP
