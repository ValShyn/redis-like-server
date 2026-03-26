#pragma once
#include <string>
#include <vector>

class CommandProcessor {
public:
    std::string execute(const std::vector<std::string>& command);
};