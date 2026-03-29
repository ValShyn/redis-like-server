#pragma once
#include <string>
#include <vector>

class CommandProcessor {
public:
    /**
     * Takes a vector of command parts and returns a RESP-formatted response string.
     */
    std::string execute(const std::vector<std::string>& command);
};