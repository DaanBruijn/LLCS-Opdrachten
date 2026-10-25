#pragma once

#include <SFML/Network.hpp>
#include <cstdint>
#include <string>
#include <vector>

enum class MessageType : std::uint8_t
{
    Join = 1,
    Leave,
    Chat,
    PrivateMessage,
    ServerEvent,
    Error,
    UserList
};

struct ChatMessage
{
    MessageType type = MessageType::Chat;
    std::string username;
    std::string recipient;
    std::string text;
    std::vector<std::string> users;
};

inline sf::Packet& operator<<(sf::Packet& packet, const ChatMessage& message)
{
    packet << static_cast<std::uint8_t>(message.type) << message.username << message.recipient << message.text;
    packet << static_cast<std::uint32_t>(message.users.size());

    for (const std::string& user : message.users)
    {
        packet << user;
    }

    return packet;
}

inline bool decodeMessage(sf::Packet& packet, ChatMessage& message)
{
    std::uint8_t type = 0;
    std::uint32_t userCount = 0;

    if (!(packet >> type >> message.username >> message.recipient >> message.text >> userCount))
    {
        return false;
    }

    if (type < static_cast<std::uint8_t>(MessageType::Join) || type > static_cast<std::uint8_t>(MessageType::UserList))
    {
        return false;
    }

    if (userCount > 1000)
    {
        return false;
    }

    message.type = static_cast<MessageType>(type);
    message.users.clear();
    message.users.reserve(userCount);

    for (std::uint32_t i = 0; i < userCount; ++i)
    {
        std::string user;

        if (!(packet >> user))
        {
            return false;
        }

        message.users.push_back(std::move(user));
    }

    return packet.endOfPacket();
}
