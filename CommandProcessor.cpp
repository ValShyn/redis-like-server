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

    if(cmd == "SET" || cmd == "set"){

        // SET command requires 3 arguments
        if(command.size() != 3){
            return "-ERR wrong number of arguments for 'set' command\r\n";
        }

        // if all is good, we set new value
        this -> db.set(command[1], command[2]);
        return "+OK\r\n";
    }

    if(cmd == "GET" || cmd == "get"){
        // GET command requires 2 arguments
        if(command.size() != 2){
            return "-ERR wrong number of arguments for 'get' command\r\n";
        }
        const auto value_from_db = this -> db.get(command[1]);
        if(value_from_db.empty()){
            // RESP format to handle the elements, which were not found in db
            return "$-1\r\n";
        }

        // RESP format: first length of value, then value
        return "$" + std::to_string(value_from_db.length()) + "\r\n" + value_from_db + "\r\n";
    }

    // Default response for unimplemented or unknown commands
    return "-ERR command not implemented yet\r\n";
}