#include "Database.hpp"


// Sets a new value for the key and clears any existing expiration time (TTL)
void Database::set(const std::string& key, const std::string& value) {
    // tool which closes the lock
    // prevents deadlocks as it is destroyed even in exceptions
    std::lock_guard<std::mutex> lock(this -> db_mutex);

    // Overwriting with a new DbValue ensures that has_expiration resets to false
    this->db[key] = {value, false, {}};
}


// Retrieves the value for a given key if it exists and hasn't expired.
// If the key has expired, it is erased from the database.
// Returns the value string, or an empty string if not found or expired.
std::string Database::get(const std::string& key){
    std::lock_guard<std::mutex> lock(this -> db_mutex);

    auto item = this -> db.find(key);

    // If the iterator hasn't reached the end of the map, it means it found a key
    if(item != this -> db.end()){

        // if there is no limit or limit is not reached - we can return value
        if(!item -> second.has_expiration || item -> second.expires_at > std::chrono::steady_clock::now()){
            return item -> second.value;
        }

        // if the limit is reached - erase key from db
        else{
            db.erase(item);
        }
    }
    return "";
}


// returns 1, if the key was deleted, otherwise - 0
bool Database::del(const std::string& key){
    std::lock_guard<std::mutex> lock(this -> db_mutex);

    // erase() returns the num of keys, which were erased
    bool is_erased = this -> db.erase(key);

    return is_erased;
}


// Returns true if the key exists and has not expired.
// Performs lazy expiration: if a key is found but expired, it is erased.
bool Database::exists(const std::string& key){
    std::lock_guard<std::mutex> lock(this -> db_mutex);

    auto item = this -> db.find(key);

    // check if the key is in db
    if(item != db.end()){

        // if key has no time limit or has not reached it - return true
        if(!item -> second.has_expiration || item -> second.expires_at > std::chrono::steady_clock::now()){
            return true;
        }

        // if key is expired - erase it
        else{
            db.erase(item);
        }
    }
    return false;
}


// if the key is not in db - adds it to db with value of 1
// if the key is in db and has expired - adds it to db with val of 1 and without expiration time
// otherwise tries to convert value to long long and increments it
// otherwise throws runtime_error if value is not convertable
long long Database::incr(const std::string& key){
    std::lock_guard<std::mutex> lock(this -> db_mutex);

    auto elem = this -> db.find(key);

    if(elem != this -> db.end()){

        // if key has expired - erase it and put iterator to the end of db
        if(elem -> second.has_expiration && elem -> second.expires_at <= std::chrono::steady_clock::now()){
            this -> db.erase(elem);
            elem = this -> db.end();
        }
    }

    // if key was not found or was erased - add it to db with value 1 and with no exparation time
    if(elem == this -> db.end()){
        this->db[key] = {"1", false, {}};
        return 1;
    }

    try{

        // try to convert value to long long
        long long elem_value = std::stoll(elem -> second.value);
        elem_value++;   // increment it

        // set a new value but save the ttl
        elem -> second.value = std::to_string(elem_value);
        return elem_value;
    }
    catch (const std::exception&) {
        throw std::runtime_error("not an int");
    }
}


// returns a vector containing all keys present in the database
// removes expired keys during iteration
std::vector<std::string> Database::keys(void){
    std::lock_guard<std::mutex> lock(this -> db_mutex);

    std::vector<std::string> result;

    // reserve as much space as there is in db
    result.reserve(this -> db.size());

    auto elem = this -> db.begin();

    while(elem != this -> db.end()){
        if(elem -> second.has_expiration && elem -> second.expires_at <= std::chrono::steady_clock::now()){
            // erase returns the iterator to the next element
            elem = this -> db.erase(elem);
        }
        else{
            result.push_back(elem -> first);
            elem++;
        }
    }

    return result;
}
// returns the remaining time to live (TTL) of a key in seconds.
// returns "-2" if the key does not exist or has expired (lazy deletion).
// returns "-1" if the key exists but has no associated expiration time.
// otherwise, returns the number of seconds until the key expires.
std::string Database::ttl(const std::string& key){
    std::lock_guard<std::mutex> lock(this -> db_mutex);

    auto elem = this -> db.find(key);

    // if the key is not found - return "-2"
    if(elem == this -> db.end()){
        return "-2";
    }

    if(elem -> second.has_expiration){
        auto now = std::chrono::steady_clock::now();
        if(elem -> second.expires_at <= now){
            this -> db.erase(elem); // lazy expiration for the key
            return "-2";    // return "-2" if the key has expired
        }

        auto duration = elem -> second.expires_at - now;
        auto seconds = std::chrono::duration_cast<std::chrono::seconds>(duration).count();

        return std::to_string(seconds); // return remaining time
    }

    return "-1";    // return "-1" if the key has no expiration time
}

// sets a time to live (TTL) for a specific key
// if the key exists and is not expired, sets the expiration time and returns true
// if the key does not exist or has already expired, returns false
bool Database::expire(const std::string& key, long long seconds){
    std::lock_guard<std::mutex> lock(this -> db_mutex);

    auto elem = this -> db.find(key);

    if(elem == this -> db.end()) return false;

    if (elem->second.has_expiration && elem->second.expires_at <= std::chrono::steady_clock::now()) {
        this->db.erase(elem);
        return false;
    }

    elem -> second.has_expiration = true;
    elem -> second.expires_at = std::chrono::steady_clock::now() + std::chrono::seconds(seconds);

    return true;
}

Database::Database(void){
    this -> background_thread = std::thread(&Database::background_cleanup, this);
}

Database::~Database(void){
    this -> should_stop = true; // sygnal for thread to stop

    // if thread was activated and still lives in memory
    // wait until it ends his loop and destroy it
    if(this->background_thread.joinable()){

        // wait for background thread to end his loop
        this->background_thread.join();
    }
}