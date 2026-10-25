#pragma once

#include "Opdracht4Client.hpp"

#include <string>
#include <vector>

class Opdracht4
{
public:
    void update();
    void render();

private:
    void addMessage(const std::string& message);

    Opdracht4Client client;

    char serverIp[64] = "127.0.0.1";
    char serverPort[16] = "53000";
    char username[32] = "Gebruiker";
    char messageInput[512] = "";
    char recipientInput[32] = "";

    bool connected = false;

    std::vector<std::string> messages;
};
