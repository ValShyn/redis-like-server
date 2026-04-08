#include "RespParser.hpp"

std::vector<std::string> RespParser::parse(const std::string& rawMessage){
    std::vector<std::string> parsedCommands;
    if (rawMessage.empty() || rawMessage[0] != '*') {
        std::stringstream ss(rawMessage);
        std::string word;
        while(ss >> word) {
            parsedCommands.push_back(word);
        }
        return parsedCommands; 
    }


    size_t cursor = 1;
    
    size_t endLine = rawMessage.find("\r\n", cursor);//find first \r\n

    //if (endLine == std::string::npos) return parsedCommands; 

    size_t length = endLine - cursor;

    int numArgs = std::stoi(rawMessage.substr(cursor, length));//to know number of arguments

    cursor = endLine + 2; //jump ober \r\n

    for(int i = 0; i < numArgs; ++i){
        if(cursor >= rawMessage.length() || rawMessage[cursor] != '$') break;
        cursor++;

        endLine = rawMessage.find("\r\n", cursor);
        if(endLine == std::string::npos) break;
        int wordLen = std::stoi(rawMessage.substr(cursor,endLine - cursor));
        cursor = endLine + 2;

        std::string word = rawMessage.substr(cursor, wordLen);
        parsedCommands.push_back(word);
        cursor += wordLen + 2;// jump over the word and \r\n
    }
    

}