#include "../../include/siyi/helper/log_manager.hpp"

namespace SIYI
{
    namespace helper
    {
        LogManager::LogManager(std::string projectName) : ILogManager(std::move(projectName)) {}

        const char* LogManager::ToText(const MessageType type) const
        {
            switch (type)
            {
                case MessageType::DEBUG:
                    return "DEBUG";
                case MessageType::INFO:
                    return "INFO";
                case MessageType::ERROR:
                    return "ERROR";
                case MessageType::WARNING:
                    return "WARNING";
            }

            return "UNKNOWN";
        }

        void LogManager::Print(const MessageType type, const std::string& message) const
        {
            std::cout << '[' << ToText(type) << "] [" << projectName_ << "]: " << message << std::endl;
        }

        std::shared_ptr<ILogManager> CreateConsoleLogger(std::string projectName)
        {
            return std::make_shared<LogManager>(std::move(projectName));
        }
    } // namespace helper
} // namespace SIYI