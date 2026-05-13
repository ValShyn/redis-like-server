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
        try{
            const auto value_from_db = this -> db.get(command[1]);
            if(value_from_db.empty()){
                // RESP format to handle the elements, which were not found in db
                return "$-1\r\n";
            }

            // RESP format: first length of value, then value
            return "$" + std::to_string(value_from_db.length()) + "\r\n" + value_from_db + "\r\n";
        }
        catch(std::invalid_argument& e){
            return "-WRONGTYPE Operation against a key holding the wrong kind of value\r\n";
        }
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

        // catch the error when string is unconvertable to long long
        // or when string is too big for long long
        catch(const std::runtime_error&){
            return "-ERR value is not an integer or out of range of long long type\r\n";
        }

        // catch the error when the value is not a string
        catch(const std::invalid_argument&){
            return "-WRONGTYPE Operation against a key holding the wrong kind of value\r\n";
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

    if(cmd == "lpush" || cmd == "LPUSH"){
        // LPUSH command requires 3 args - lpush key value
        if(command.size() != 3){
            return "-ERR wrong number of arguments for 'lpush' command\r\n";
        }

        try{
            std::size_t deque_size = this -> db.lpush(command[1], command[2]);
            return ":" + std::to_string(deque_size) + "\r\n";
        }
        catch(std::invalid_argument&){
            return "-WRONGTYPE Operation against a key holding the wrong kind of value\r\n";
        }
    }

    if(cmd == "rpush" || cmd == "RPUSH"){
        // RPUSH command requires 3 args - rpush key value
        if(command.size() != 3){
            return "-ERR wrong number of arguments for 'rpush' command\r\n";
        }

        try{
            std::size_t deque_size = this -> db.rpush(command[1], command[2]);
            return ":" + std::to_string(deque_size) + "\r\n";
        }
        catch(std::invalid_argument&){
            return "-WRONGTYPE Operation against a key holding the wrong kind of value\r\n";
        }
    }

    if(cmd == "llen" || cmd == "LLEN"){
        // LLEN command requires 2 args - llen key
        if(command.size() != 2){
            return "-ERR wrong number of arguments for 'llen' command\r\n";
        }

        try{
            std::size_t deque_size = this -> db.llen(command[1]);
            return ":" + std::to_string(deque_size) + "\r\n";
        }
        catch(std::invalid_argument&){
            return "-WRONGTYPE Operation against a key holding the wrong kind of value\r\n";
        }
    }

    if(cmd == "lpop" || cmd == "LPOP"){
        // lpop key
        if(command.size() != 2){
            return "-ERR wrong number of arguments for 'lpop' command\r\n";
        }

        try {
            std::string val = this->db.lpop(command[1]);

            // if pop_internal returned empty string, it means key doesn't exist
            if(val.empty()){
                return "$-1\r\n";
            }

            // return as Bulk String: $len\r\nvalue\r\n
            return "$" + std::to_string(val.length()) + "\r\n" + val + "\r\n";
        }
        catch(const std::invalid_argument&){
            return "-WRONGTYPE Operation against a key holding the wrong kind of value\r\n";
        }
    }

    if(cmd == "rpop" || cmd == "RPOP"){
        // rpop key
        if(command.size() != 2){
            return "-ERR wrong number of arguments for 'rpop' command\r\n";
        }

        try {
            std::string val = this->db.rpop(command[1]);

            if(val.empty()){
                return "$-1\r\n";
            }

            return "$" + std::to_string(val.length()) + "\r\n" + val + "\r\n";
        }
        catch(const std::invalid_argument&){
            return "-WRONGTYPE Operation against a key holding the wrong kind of value\r\n";
        }
    }

    if(cmd == "lindex" || cmd == "LINDEX"){
        // lindex key index
        if(command.size() !=3){
            return "-ERR wrong number of arguments for 'lindex' command\r\n";
        }

        try {
            // convert string to long long before passing by value to lindex
            long long idx = std::stoll(command[2]);
            std::string val = this->db.lindex(command[1], idx);

            if(val.empty()){
                return "$-1\r\n";
            }
            return "$" + std::to_string(val.length()) + "\r\n" + val + "\r\n";
        }
        catch (const std::invalid_argument& e) {
            std::string msg = e.what();
            // check, if it is the error from db or from long long convertation
            if (msg.find("wrong kind of value") != std::string::npos) {
                return "-WRONGTYPE Operation against a key holding the wrong kind of value\r\n";
            }
            // if the origin is not db - it is cause of long long convertation
            return "-ERR value is not an integer or out of range\r\n";
        }
        catch (const std::out_of_range&) {
            return "-ERR value is out of range\r\n";
        }
    }

    if(cmd == "lrange" || cmd == "LRANGE"){
        // lrange key start end
        if(command.size() != 4){
            return "-ERR wrong number of arguments for 'lrange' command\r\n";
        }

        try {
            long long start = std::stoll(command[2]);
            long long end = std::stoll(command[3]);

            std::vector<std::string> result = this->db.lrange(command[1], start, end);

            // RESP array starts with *
            std::string response = "*" + std::to_string(result.size()) + "\r\n";

            for(const auto& str : result){
                // each element in array is a Bulk String
                response += "$" + std::to_string(str.length()) + "\r\n" + str + "\r\n";
            }

            return response;
        }
        catch (const std::invalid_argument& e) {
            // we need to check if it's a stoll error or our DB's WRONGTYPE error
            std::string msg = e.what();

            // npos stays for no position, find() returns the position, where the string pattern starts
            // for us it means, that the error was caused not by wrong type in db, but in start or end vars
            if (msg.find("wrong kind of value") != std::string::npos) {
                return "-WRONGTYPE Operation against a key holding the wrong kind of value\r\n";
            }
            return "-ERR value is not an integer or out of range\r\n";
        }
        catch (const std::out_of_range&) {
            return "-ERR value is out of range\r\n";
        }
    }

    if(cmd == "lset" || cmd == "LSET"){
        // lset key index new_value
        if(command.size() != 4){
            return "-ERR wrong number of arguments for 'lset' command\r\n";
        }

        try {
            long long idx = std::stoll(command[2]);
            this->db.lset(command[1], idx, command[3]);
            return "+OK\r\n";
        }
        catch (const std::out_of_range&) {
            return "-ERR index out of range\r\n";
        }
        catch (const std::runtime_error&) {
            return "-ERR no such key\r\n";
        }
        catch (const std::invalid_argument& e) {
            if (std::string(e.what()).find("wrong kind of value") != std::string::npos) {
                return "-WRONGTYPE Operation against a key holding the wrong kind of value\r\n";    // error from db
            }
            return "-ERR value is not an integer\r\n"; // error from stoll()
        }
    }

    // Default response for unimplemented or unknown commands
    return "-ERR command not implemented yet\r\n";
}