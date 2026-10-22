#include "Opdracht4Client.hpp"

#include <iostream>

bool Opdracht4Client::connect(const std::string& ip,std::uint16_t port)
{
    if (connected)
        disconnect();

    const auto addresses = sf::Dns::resolve(ip);
    if (!addresses || addresses->empty())
    {
        std::cerr << "Geen IP-adres gevonden voor: " << ip << '\n';
        return false;
    }

    const sf::Socket::Status status = socket.connect(addresses->front(), port, sf::seconds(3));
    if (status != sf::Socket::Status::Done)
    {
        std::cerr << "Verbinding met server mislukt " << ip << '-' << port << '\n';
        return false;
    }

    connected = true;

    std::cout << "Verbonden met -" << ip << ':' << port << '\n';
    return true;
}

void Opdracht4Client::disconnect()
{
    socket.disconnect();
    connected = false;
}

bool Opdracht4Client::isConnected() const
{
    return connected;
}

bool Opdracht4Client::sendMessage(const std::string& message)
{
    if (!connected)
        return false;

    const sf::Socket::Status status = socket.send(message.c_str(), message.size());
    return status == sf::Socket::Status::Done;
}
