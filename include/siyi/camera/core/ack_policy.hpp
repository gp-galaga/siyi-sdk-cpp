#ifndef SIYI_ACK_POLICY_HPP
#define SIYI_ACK_POLICY_HPP

#include "../protocol/tc_parameter.hpp"

#include <cstdint>
#include <vector>

namespace SIYI
{
    namespace AckPolicy
    {
        inline uint8_t ExpectedAckCmdIdForRequest(uint8_t requestCmdId) noexcept
        {
            // Some cameras answer PHOTO_RECORD (0x0C) with FUNC_FEEDBACK_INFO (0x0B).
            if (requestCmdId == static_cast<uint8_t>(CommandId::PHOTO_RECORD))
            {
                return static_cast<uint8_t>(CommandId::FUNC_FEEDBACK_INFO);
            }
            return requestCmdId;
        }

        inline bool IsExpectedAckCmdIdForRequest(uint8_t responseCmdId, uint8_t requestCmdId) noexcept
        {
            return responseCmdId == ExpectedAckCmdIdForRequest(requestCmdId);
        }

        // The SIYI wire protocol carries the ack requirement in the request frame's own
        // control byte (index 2): NO_ACK means the device is under no obligation to reply
        // at all. PHOTO_RECORD sub-functions such as motion mode, video output, and HDR
        // are built with NO_ACK (see SharedCameraModel::ControlPhotoRecord) and, on real
        // ZR30 hardware, never produce a response; only TAKE_PICTURE / START_STOP_RECORDING
        // occasionally trigger an unsolicited FUNC_FEEDBACK_INFO event. A caller that blocks
        // waiting for a reply to a NO_ACK request will either time out or, if some unrelated
        // packet happens to arrive in that window, misreport it as an error. This helper lets
        // callers tell the two situations apart from a genuine NEED_ACK request that got no
        // (or the wrong) reply.
        inline bool RequestFrameExpectsAck(const std::vector<uint8_t>& requestFrame) noexcept
        {
            constexpr std::size_t kCtrlIndex = 2;
            if (requestFrame.size() <= kCtrlIndex)
            {
                return true;
            }
            return requestFrame[kCtrlIndex] == static_cast<uint8_t>(ControlFlag::NEED_ACK);
        }
    } // namespace AckPolicy
} // namespace SIYI

#endif // SIYI_ACK_POLICY_HPP
