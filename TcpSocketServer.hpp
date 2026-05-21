#pragma once

#include "CommandProcessor.hpp"

bool handleClientData(int clientSocket, CommandProcessor& processor);

void runEventLoop(int serverSocket, CommandProcessor& processor);

