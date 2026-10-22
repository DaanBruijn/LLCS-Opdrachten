#pragma once

#include "Opdracht4Protocol.hpp"
#include <SFML/Network.hpp>
#include <cstdint>
#include <string>

class Opdracht4Client
{
public:
    bool connect(const std::string& ip, std::uint16_t port, const std::string& username);
    void disconnect();

    bool isConnected() const;

    bool sendMessage(const ChatMessage& message);
    bool receiveMessage(ChatMessage& message);

private:
    sf::TcpSocket socket;
    bool connected = false;
};
