#include "Opdracht4Server.hpp"

#include <algorithm>
#include <iostream>
#include <utility>

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
        std::cerr << "Server kon niet luisteren op poort - " << port << '\n';
        return false;
    }

    listener.setBlocking(false);

    running.store(true);
    serverThread = std::thread(&Opdracht4Server::run, this);

    std::cout << "Chatserver gestart op poort - " << port << '\n';
    return true;
}

void Opdracht4Server::stop()
{
    if (!running.exchange(false))
        return;

    listener.close();

    if (serverThread.joinable())
        serverThread.join();

    std::lock_guard<std::mutex> lock(clientsMutex);
    clients.clear();

    std::cout << "Chatserver gestopt. Womp Womp\n";
}

bool Opdracht4Server::isRunning() const
{
    return running.load();
}

void Opdracht4Server::run()
{
    while (running.load())
    {
        auto socket = std::make_unique<sf::TcpSocket>();
        const sf::Socket::Status acceptStatus = listener.accept(*socket);

        if (!running.load())
            break;

        if (acceptStatus == sf::Socket::Status::Done)
        {
            socket->setBlocking(false);

            ClientInfo client;
            client.socket = std::move(socket);

            std::lock_guard<std::mutex> lock(clientsMutex);
            clients.push_back(std::move(client));

            std::cout << "Nieuwe client verbonden. Totaal - " << clients.size() << '\n';
        }

        {
            std::lock_guard<std::mutex> lock(clientsMutex);

            for (std::size_t i = 0; i < clients.size();)
            {
                ChatMessage message;
                sf::Packet packet;

                const sf::Socket::Status status = clients[i].socket->receive(packet);

                if (status == sf::Socket::Status::Done)
                {
                    if (decodeMessage(packet, message))
                    {
                        processClientMessage(i, message);
                    }
                    if (i < clients.size())
                        ++i;
                }
                else if (status == sf::Socket::Status::Disconnected)
                {
                    const std::string username =
                        clients[i].username;

                    clients.erase(clients.begin() +
                                  static_cast<std::ptrdiff_t>(i));

                    if (!username.empty())
                    {
                        ChatMessage event;
                        event.type = MessageType::ServerEvent;
                        event.username = "Server";
                        event.text = username + " heeft de chat verlaten. Goodbye! - " + username;

                        broadcast(event);
                        broadcastUserList();
                    }

                    std::cout << "Client disconnected\n";
                }
                else
                {
                    ++i;
                }
            }
        }

        sf::sleep(sf::milliseconds(5));
    }
}

void Opdracht4Server::processClientMessage(std::size_t clientIndex, const ChatMessage& message)
{
    if (clientIndex >= clients.size())
        return;

    ClientInfo& client = clients[clientIndex];

    if (message.type == MessageType::Join)
    {
        if (message.username.empty() ||
            message.username.size() > 31)
        {
            ChatMessage error;
            error.type = MessageType::Error;
            error.username = "Server";
            error.text = "Ongeldige gebruikersnaam";

            sf::Packet packet;
            packet << error;

            const sf::Socket::Status sendStatus = client.socket->send(packet);

            if (sendStatus != sf::Socket::Status::Done)
            {
                std::cerr << "Fout bij versturen van foutmelding.. womp\n";
            }
            return;
        }

        client.username = message.username;

        ChatMessage event;
        event.type = MessageType::ServerEvent;
        event.username = "Server";
        event.text = client.username + " is de chat gejoined!";

        std::cout << event.text << '\n';
        broadcast(event);
        broadcastUserList();

        return;
    }

    if (client.username.empty())
        return;

    if (message.type == MessageType::Chat)
    {
        ChatMessage outgoing = message;
        outgoing.username = client.username;
        outgoing.recipient.clear();

        broadcast(outgoing);
        return;
    }

    if (message.type == MessageType::PrivateMessage)
    {
        ChatMessage outgoing = message;
        outgoing.username = client.username;

        for (ClientInfo& recipient : clients)
        {
            if (recipient.username == message.recipient)
            {
                sf::Packet packet;
                packet << outgoing;
                recipient.socket->send(packet);
                return;
            }
        }

        ChatMessage error;
        error.type = MessageType::Error;
        error.username = "Server";
        error.text = "Gebruiker niet gevonden - " + message.recipient;

        sf::Packet packet;
        packet << error;
        client.socket->send(packet);
    }
}

void Opdracht4Server::broadcast(const ChatMessage& message, std::size_t excludedClient)
{
    sf::Packet packet;
    packet << message;

    for (std::size_t i = 0; i < clients.size(); ++i)
    {
        if (i == excludedClient)
            continue;

        clients[i].socket->send(packet);
    }
}


void Opdracht4Server::broadcastUserList()
{
    ChatMessage message;
    message.type = MessageType::UserList;
    message.username = "Server";

    for (const ClientInfo& client : clients)
    {
        if (!client.username.empty())
        {
            message.users.push_back(client.username);
        }
    }

    sf::Packet packet;
    packet << message;

    for (ClientInfo& client : clients)
    {
        if (client.username.empty())
        {
            continue;
        }

        const sf::Socket::Status status = client.socket->send(packet);

        if (status != sf::Socket::Status::Done)
        {
            std::cerr << "Online gebruikersslijst versturen niet gelukt\n";
        }
    }
}
