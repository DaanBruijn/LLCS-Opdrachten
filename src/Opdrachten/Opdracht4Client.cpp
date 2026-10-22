#include "Opdracht4Client.hpp"

#include <iostream>

bool Opdracht4Client::connect(const std::string& ip, std::uint16_t port, const std::string& username)
{
    if (connected)
        disconnect();

    const auto address = sf::IpAddress::resolve(ip);

    if (!address)
    {
        std::cerr << "Geen geldig IP-adres gevonden voor - " << ip << '\n';
        return false;
    }

    if (socket.connect(*address, port, sf::seconds(3)) != sf::Socket::Status::Done)
    {
        std::cerr << "Verbinding mislukt\n";
        return false;
    }

    socket.setBlocking(false);
    connected = true;

    ChatMessage joinMessage;
    joinMessage.type = MessageType::Join;
    joinMessage.username = username;

    if (!sendMessage(joinMessage))
    {
        disconnect();
        std::cerr << "Aanmelden bij de server mislukt\n";
        return false;
    }

    std::cout << "Verbonden met " << ip << ' - ' << port << " als - " << username << '\n';
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

bool Opdracht4Client::sendMessage(const ChatMessage& message)
{
    if (!connected)
        return false;

    sf::Packet packet;
    packet << message;

    const sf::Socket::Status status = socket.send(packet);

    if (status == sf::Socket::Status::Done)
        return true;

    if (status == sf::Socket::Status::Disconnected)
        disconnect();

    if (status == sf::Socket::Status::Partial)
    {
        std::cerr << "Pakket gedeeltelijk verstuurd\n";
    }

    return false;
}

bool Opdracht4Client::receiveMessage(ChatMessage& message)
{
    if (!connected)
        return false;

    sf::Packet packet;
    const sf::Socket::Status status = socket.receive(packet);

    if (status == sf::Socket::Status::Done)
    {
        if (!decodeMessage(packet, message))
        {
            std::cerr << "Ongeldig chatbericht ontvangen\n";
            return false;
        }

        return true;
    }

    if (status == sf::Socket::Status::Disconnected)
    {
        disconnect();
        std::cout << "verbindingg met de server verbroke...\n";
    }

    return false;
}
