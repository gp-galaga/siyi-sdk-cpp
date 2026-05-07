#include "../../include/helper/ilog_manager.hpp"

namespace SIYI
{
    namespace helper
    {
        void LogManager::PrintError(const std::string& message) const
        {
            Print(MessageType::ERROR, message);
        }

        void LogManager::PrintWarning(const std::string& message) const
        {
            Print(MessageType::WARNING, message);
        }

        void LogManager::PrintInfo(const std::string& message) const
        {
            Print(MessageType::INFO, message);
        }

        void LogManager::PrintDebug(const std::string& message) const
        {
            Print(MessageType::DEBUG, message);
        }

        void LogManager::Print(const MessageType type, const std::string& message) const
        {
            std::cout << '[' << ToText(type) << "] " << this->projectName_ << ": " << message << std::endl;
        }

        const char* LogManager::ToText(const MessageType type)
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
    } // namespace helper
} // namespace SIYI