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

    if(cmd == "del" || cmd == "DEL"){
        // DEL requires at least one key
        if(command.size() != 2){
            return "-ERR wrong number of arguments for 'del' command\r\n";
        }

        const bool is_deleted = this -> db.del(command[1]);

        //Redis return nums of deleted keys in DEL operation
        if(is_deleted){
            return ":" + std::to_string(is_deleted) + "\r\n";
        }
        return ":0\r\n";
    }

    if(cmd == "exists" || cmd == "EXISTS"){
        // EXISTS command requires two arguments
        if(command.size() != 2){
            return "-ERR wrong number of arguments for 'exists' command\r\n";
        }

        bool exists = this -> db.exists(command[1]);

        // if key exists, return 1, else 0
        if(exists){
            return ":1\r\n";
        }

        return ":0\r\n";
    }

    if(cmd == "incr" || cmd == "INCR"){
        // INCR command requires two arguments
        if(command.size() != 2){
            return "-ERR wrong number of arguments for 'incr' command\r\n";
        }

        try{
            long long increased_value = this -> db.incr(command[1]);
            return ":" + std::to_string(increased_value) + "\r\n";
        }
        catch(std::runtime_error&){
            return "-ERR value is not an integer or out of range\r\n";
        }
    }

    if(cmd == "keys" || cmd == "KEYS"){
        if(command.size() != 2){
            return "-ERR wrong number of arguments for 'keys' command\r\n";
        }

        if(command[1] != "*"){
            return "-ERR 'keys' command requires '*'\r\n";
        }

        std::vector<std::string> result = this -> db.keys();

        // response starts with the number of keys
        std::string response = "*" + std::to_string(result.size()) + "\r\n";

        for(const auto& key : result){

            // for each key we add his size and key
            response += "$" + std::to_string(key.length()) + "\r\n" + key + "\r\n";
        }

        return response;

    }

    if(cmd == "ttl" || cmd == "TTL"){
        if(command.size() != 2){
            return "-ERR wrong number of arguments for 'ttl' command\r\n";
        }

        std::string key = command[1];
        if (key.empty()) {
            return "-ERR 'ttl' command requires a non-empty key\r\n";
        }

        // return how many seconds are left
        // -2 means that key does not exist or has been expired
        // -1 means key has no expiraton time
        return ":" + this -> db.ttl(key) + "\r\n";
    }

    if(cmd == "expire" || cmd == "EXPIRE"){
        // validate arguments: EXPIRE key seconds
        if(command.size() != 3){
            return "-ERR wrong number of arguments for 'expire' command\r\n";
        }
        std::string key = command[1];

        try{
            // convert string duration to long long
            long long seconds = std::stoll(command[2]);
            bool was_added_to_db = this -> db.expire(key, seconds); // update db and return status

            // return RESP format: 1 - success, 0 - failed
            return ":" + std::to_string(was_added_to_db) + "\r\n";
        }
        catch(std::invalid_argument){
            return "-ERR value is not an integer\r\n";
        }
        catch(std::out_of_range){
            return "-ERR number is too big for long long\r\n";
        }

    }

    // Default response for unimplemented or unknown commands
    return "-ERR command not implemented yet\r\n";
}