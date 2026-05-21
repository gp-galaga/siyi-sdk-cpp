#ifndef __SIYI_LOG_MANAGER_HPP__
#define __SIYI_LOG_MANAGER_HPP__

#include <iostream>
#include <memory>
#include <string>
#include <utility>

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

        class ILogManager
        {
        public:
            explicit ILogManager(std::string projectName = "SIYI") : projectName_(std::move(projectName)) {}
            virtual ~ILogManager() = default;

            void PrintError(const std::string& message) const
            {
                Print(MessageType::ERROR, message);
            }

            void PrintWarning(const std::string& message) const
            {
                Print(MessageType::WARNING, message);
            }

            void PrintInfo(const std::string& message) const
            {
                Print(MessageType::INFO, message);
            }

            void PrintDebug(const std::string& message) const
            {
                Print(MessageType::DEBUG, message);
            }

        protected:
            std::string projectName_;
            virtual void Print(MessageType type, const std::string& message) const = 0;
            virtual const char* ToText(MessageType type) const = 0;
        };

        std::shared_ptr<ILogManager> CreateConsoleLogger(std::string projectName);
    } // namespace helper
} // namespace SIYI

#endif
