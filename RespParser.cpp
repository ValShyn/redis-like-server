#include "RespParser.hpp"
#include <sstream>

std::vector<std::string> RespParser::parse(const std::string& rawMessage){
    std::vector<std::string> parsedCommands;
    //that's a method if the client send string instead of protocol
    if (rawMessage.empty() || rawMessage[0] != '*') {
        std::stringstream ss(rawMessage);
        std::string word;
        while(ss >> word) {
            parsedCommands.push_back(word);
        }
        return parsedCommands; 
    }

    //create cursor to understand where we are
    size_t cursor = 1;
    
    size_t endLine = rawMessage.find("\r\n", cursor);//find first \r\n from cursor

    if (endLine == std::string::npos) return parsedCommands; //if endLine is Nun we return command

    size_t length = endLine - cursor;
    /** 
    take the number of arguments , for ex.,*2\r\n$5\r\nhello\r\n$5\r\nworld\r\n, * means array, 2 means size of array,
    $ means string , the number 5 after it means lenght of this string 
    and : means integer
    */
    int numArgs = std::stoi(rawMessage.substr(cursor, length));//to know number of arguments

    cursor = endLine + 2; //jump over \r\n
    //loop through all elements of the command
    for(int i = 0; i < numArgs; ++i){
        //the first element is always $
        if(cursor >= rawMessage.length() || rawMessage[cursor] != '$') break;
        cursor++;//jump over $
        
        endLine = rawMessage.find("\r\n", cursor);//check the next \r\n
        if(endLine == std::string::npos) break;//if endLine is Nun we break
        int wordLen = std::stoi(rawMessage.substr(cursor,endLine - cursor));//we convert length of the word which is string type to integer with std::stoi()
        cursor = endLine + 2;

        std::string word = rawMessage.substr(cursor, wordLen);//take this word
        parsedCommands.push_back(word);
        cursor += wordLen + 2;// jump over the word and \r\n
        }

    return parsedCommands;
    

}