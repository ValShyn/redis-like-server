#include "CommandProcessor.hpp"

std::string CommandProcessor::execute(const std::vector<std::string>& command) {
    // Check if the command vector is empty to prevent crashes
    if (command.empty()) {
        return "-ERR empty command\r\n";
    }

    // The first element is always the command name
    const std::string cmd = command[0];

    // Handle PING command for connectivity testing
    if (cmd == "PING" || cmd == "ping") {
        return "+PONG\r\n";
    }

    // Default response for unimplemented or unknown commands
    return "-ERR command not implemented yet\r\n";
}