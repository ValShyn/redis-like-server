#include "Database.hpp"

void Database::set(const std::string& key, const std::string& value){
    this -> db[key] = value;
}

std::string Database::get(const std::string& key) const{
    auto item = this -> db.find(key);

    // If the iterator hasn't reached the end of the map, it means it found a key
    if(item != this -> db.end()){
        return item -> second;
    }
    return "";
}

bool Database::del(const std::string& key){
    // erase() returns the num of keys, which were erased
    bool is_erased = this -> db.erase(key);

    return is_erased;
}

bool Database::exists(const std::string& key) const{
    auto item = this -> db.find(key);

    // return true if the key exists in db
    if(item != db.end()){
        return true;
    }
    return false;
}

long long Database::incr(const std::string& key){
    auto elem = this -> db.find(key);

    // if key is not found, add it to db with value of 1
    if(elem == this -> db.end()){
        set(key, "1");
        return 1;
    }

    // if the key is found and is convertable to long long
    // increase it and return
    try{
        // std::stoll() to convert string to long long
        long long elem_value = std::stoll(elem -> second);
        set(key, std::to_string(++elem_value));
        return elem_value;
    }

    // throw an error if we catch an error
    catch(std::out_of_range&){
        throw std::runtime_error("not an int");
    }
    catch(std::invalid_argument&){
        throw std::runtime_error("not an int");
    }
}