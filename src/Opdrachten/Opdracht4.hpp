#pragma once

#include "Opdracht4Client.hpp"

#include <chrono>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

class Opdracht4
{
public:
    void update();
    void render();

private:
    struct ChatEntry
    {
        std::uint32_t id = 0;
        std::uint32_t replyToId = 0;
        std::string username;
        std::string text;
        bool isPrivate = false;
    };

    void addMessage(const std::string& message);

    Opdracht4Client client;

    char serverIp[64] = "127.0.0.1";
    char serverPort[16] = "53000";
    char username[32] = "Gebruiker";
    char messageInput[512] = "";
    char recipientInput[32] = "";

    bool connected = false;

    std::vector<std::string> messages;
    std::vector<std::string> onlineUsers;
    std::vector<ChatEntry> chatEntries;

    std::uint32_t replyToId = 0;

    std::unordered_map<std::string, std::chrono::steady_clock::time_point > typingUsers;

    bool lastTypingState = false;
    std::chrono::steady_clock::time_point lastTypingSent{};
};
