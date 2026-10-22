#include "Opdracht4Server.hpp"

#include <iostream>
#include <memory>

Opdracht4Server::~Opdracht4Server()
{
    stop();
}

bool Opdracht4Server::start(std::uint16_t port)
{
    if (running.load())
        return false;

    if (listener.listen(port) != sf::Socket::Status::Done)
    {
        std::cerr << "Server kon niet luisteren op poort -" << port << '\n';
        return false;
    }

    running.store(true);
    acceptThread = std::thread(&Opdracht4Server::acceptClients, this);
    std::cout << "Chatserver gestart op poort -" << port << '\n';

    listener.setBlocking(false);

    return true;
}

void Opdracht4Server::stop()
{
    if (!running.exchange(false))
        return;

    listener.close();

    if (acceptThread.joinable())
        acceptThread.join();

    std::lock_guard<std::mutex> lock(clientsMutex);
    clients.clear();

    std::cout << "Chatserver gestopt\n";
}

bool Opdracht4Server::isRunning() const
{
    return running.load();
}

void Opdracht4Server::acceptClients()
{
    while (running.load())
    {
        auto client = std::make_unique<sf::TcpSocket>();
        const sf::Socket::Status status = listener.accept(*client);

        if (!running.load())
            break;

        if (status == sf::Socket::Status::Done)
        {
            client->setBlocking(false);
            std::size_t clientCount = 0;
            {
                std::lock_guard<std::mutex> lock(clientsMutex);
                clients.push_back(std::move(client));
                clientCount = clients.size();
            }

            std::cout << "Nieuwe client verbonden - Totaal: " << clientCount << '\n';
        }
        else if (status == sf::Socket::Status::NotReady)
        {
            sf::sleep(sf::milliseconds(10));
        }
        else
        {
            if (running.load())
                std::cerr << "Fout bij accepteren van client\n";
        }
    }
}