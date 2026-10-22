#pragma once

#include <SFML/Network.hpp>

#include <cstdint>
#include <string>

class Opdracht4Client
{
public:
    bool connect(const std::string& ip, std::uint16_t port);
    void disconnect();

    bool isConnected() const;
    bool sendMessage(const std::string& message);

private:
    sf::TcpSocket socket;
    bool connected = false;
};
