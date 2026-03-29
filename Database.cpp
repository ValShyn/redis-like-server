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