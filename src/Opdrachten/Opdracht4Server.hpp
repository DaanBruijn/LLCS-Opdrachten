#pragma once

#include <SFML/Network.hpp>

#include <atomic>
#include <cstdint>
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
    void acceptClients();

    sf::TcpListener listener;
    std::thread acceptThread;

    std::atomic<bool> running{false};
    std::mutex clientsMutex;
    std::vector<std::unique_ptr<sf::TcpSocket>> clients;
};