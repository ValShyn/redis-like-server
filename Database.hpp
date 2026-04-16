#pragma once
#include <unordered_map>
#include <string>
#include <stdexcept>
#include <vector>
#include <chrono>
#include <mutex>    // to protect shared data from synchronous access by multiple threads
#include <thread>   // to add background thread which will clean up expired keys


// struct which describes value in our db
// taking into account TTL(time to live) keys
// not every key has expiration time - trace this with bool value
// describe time with chrono module
struct DbValue{
    std::string value;
    bool has_expiration = false;
    std::chrono::steady_clock::time_point expires_at;
};

class Database{
    private:
    std::unordered_map<std::string, DbValue> db;

    bool should_stop = false;   // condition on which clean up thread will stop his work
    std::thread background_thread;
    void background_cleanup(void);  // private function which will provide a cleanup of expired keys

    // acts as a lock for the 'db' map
    // it prevents race conditions by ensuring that the main server thread
    // and the background cleanup thread cannot modify the map at the same time
    std::mutex db_mutex;

    public:
    void set(const std::string& key, const std::string& value);
    std::string get(const std::string& key);
    bool del(const std::string& key);
    bool exists(const std::string& key);
    long long incr(const std::string& key);
    std::vector<std::string> keys(void);
    std::string ttl(const std::string& key);
    bool expire(const std::string& key, long long seconds);
    Database(void);
    ~Database(void);
};