#pragma once

#include "Opdracht4Protocol.hpp"

#include <SFML/Network.hpp>

#include <atomic>
#include <cstdint>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

class Opdracht4Server
{
public:
    Opdracht4Server() = default;
    ~Opdracht4Server();

    Opdracht4Server(const Opdracht4Server&) = delete;
    Opdracht4Server& operator=(const Opdracht4Server&) = delete;

    bool start(std::uint16_t port);
    void stop();

    bool isRunning() const;

private:
    struct ClientInfo
    {
        std::unique_ptr<sf::TcpSocket> socket;
        std::string username;
    };

    void run();

    void processClientMessage(std::size_t clientIndex, const ChatMessage& message);
    void broadcast(const ChatMessage& message, std::size_t excludedClient = static_cast<std::size_t>(-1));

    sf::TcpListener listener;
    std::thread serverThread;

    std::atomic<bool> running{false};

    std::mutex clientsMutex;
    std::vector<ClientInfo> clients;
};
