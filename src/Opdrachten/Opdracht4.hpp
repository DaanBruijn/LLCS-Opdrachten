#pragma once
#include <string>

class Opdracht4
{
public:
    void update();
    void render();

private:
    char serverIp[64] = "127.0.0.1";
    char serverPort[16] = "53000";
    char username[32] = "Gebruiker";
    char messageInput[512] = "";

    bool connected = false;
};