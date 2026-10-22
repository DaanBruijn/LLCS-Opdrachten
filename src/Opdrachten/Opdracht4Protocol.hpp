#pragma once
#include <SFML/Network.hpp>

#include <cstdint>
#include <string>

enum class MessageType : std::uint8_t
{
    Join = 1,
    Leave,
    Chat,
    PrivateMessage,
    ServerEvent,
    Error
};

struct ChatMessage
{
    MessageType type = MessageType::Chat;
    std::string username;
    std::string recipient;
    std::string text;
};

inline sf::Packet& operator<<(sf::Packet& packet,const ChatMessage& message)
{
    packet << static_cast<std::uint8_t>(message.type) << message.username << message.recipient << message.text;
    return packet;
}

inline bool decodeMessage(sf::Packet& packet, ChatMessage& message)
{
    std::uint8_t type = 0;

    if (!(packet >> type >> message.username >> message.recipient >> message.text))
    {
        return false;
    }

    if (type < static_cast<std::uint8_t>(MessageType::Join) || type > static_cast<std::uint8_t>(MessageType::Error))
    {
        return false;
    }

    message.type = static_cast<MessageType>(type);
    return packet.endOfPacket();
}
