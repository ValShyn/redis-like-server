#pragma once
#include <string>
#include <vector>

class RespParser {
    public:
        static std::vector<std::string> parse(const std::string& rawMessage);
};