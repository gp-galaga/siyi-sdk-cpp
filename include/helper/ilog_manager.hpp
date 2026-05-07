#ifndef __SIYI_LOG_MANAGER_HPP__
#define __SIYI_LOG_MANAGER_HPP__

#include <iostream>
#include <string>

namespace SIYI
{
    namespace helper
    {
        enum class MessageType
        {
            DEBUG,
            INFO,
            ERROR,
            WARNING
        };

        class LogManager
        {
        public:
            explicit LogManager(std::string projectName = "SIYI")
                : projectName_(std::move(projectName))
            {
            }

            void PrintError(const std::string& message) const;
            void PrintWarning(const std::string& message) const;
            void PrintInfo(const std::string& message) const;
            void PrintDebug(const std::string& message) const;

        private:
            std::string projectName_;
            void Print(MessageType type, const std::string& message) const;
            static const char* ToText(MessageType type);
        };
    } // namespace helper
} // namespace SIYI

#endif
