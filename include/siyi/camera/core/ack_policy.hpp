#ifndef SIYI_ACK_POLICY_HPP
#define SIYI_ACK_POLICY_HPP

#include "../protocol/tc_parameter.hpp"

#include <cstdint>

namespace SIYI
{
    namespace AckPolicy
    {
        inline constexpr uint8_t kFuncFeedbackInfoCmdId = 0x0B;

        inline uint8_t ExpectedAckCmdIdForRequest(uint8_t requestCmdId) noexcept
        {
            // Some cameras answer PHOTO_RECORD (0x0C) with FUNC_FEEDBACK_INFO (0x0B).
            if (requestCmdId == static_cast<uint8_t>(CommandId::PHOTO_RECORD))
            {
                return kFuncFeedbackInfoCmdId;
            }
            return requestCmdId;
        }

        inline bool IsExpectedAckCmdIdForRequest(uint8_t responseCmdId, uint8_t requestCmdId) noexcept
        {
            return responseCmdId == ExpectedAckCmdIdForRequest(requestCmdId);
        }
    } // namespace AckPolicy
} // namespace SIYI

#endif // SIYI_ACK_POLICY_HPP
