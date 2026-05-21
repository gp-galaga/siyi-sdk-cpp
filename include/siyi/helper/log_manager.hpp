#ifndef __SIYI_CONSOLE_LOG_MANAGER_HPP__
#define __SIYI_CONSOLE_LOG_MANAGER_HPP__

#include "ilog_manager.hpp"

namespace SIYI
{
    namespace helper
    {
        class LogManager : public ILogManager
        {
        public:
            explicit LogManager(std::string projectName = "SIYI");

        private:
            const char* ToText(MessageType type) const override;
            void Print(MessageType type, const std::string& message) const override;
        };
    } // namespace helper
} // namespace SIYI

#endif
