#include "Opdracht4.hpp"

#include <imgui.h>

#include <cstdint>
#include <iostream>
#include <string>

void Opdracht4::addMessage(const std::string& message)
{
    messages.push_back(message);

    // - Limit messages in window
    constexpr std::size_t maxMessages = 200;

    if (messages.size() > maxMessages)
    {
        messages.erase(messages.begin());
    }

    std::cout << message << '\n';
}

void Opdracht4::update()
{
    if (!client.isConnected())
    {
        connected = false;
        return;
    }

    ChatMessage message;
    while (client.receiveMessage(message))
    {
        switch (message.type)
        {
        case MessageType::Chat:
            addMessage(message.username + "- " + message.text);
            break;

        case MessageType::PrivateMessage:
            addMessage("[Privé] " + message.username + "- " + message.text);
            break;

        case MessageType::ServerEvent:
            addMessage("[Server] " + message.text);
            break;

        case MessageType::Error:
            addMessage("[Error] " + message.text);
            break;

        default:
            break;
        }
    }

    connected = client.isConnected();
}

void Opdracht4::render()
{
    ImGui::Begin("Opdracht4 - Chat Client");

    ImGui::Text("Verbindinginstellingen");
    ImGui::Separator();

    ImGui::InputText("Server IP", serverIp, sizeof(serverIp));
    ImGui::InputText("Poort", serverPort, sizeof(serverPort));
    ImGui::InputText("Gebruikersnaam", username, sizeof(username));

    ImGui::Spacing();

    if (!connected)
    {
        if (ImGui::Button("Verbinden"))
        {
            try
            {
                const int port = std::stoi(serverPort);

                if (port < 1 || port > 65535)
                {
                    addMessage("[Fout] Poort moet tussen 1 en 65535 liggen");
                }
                else if (username[0] == '\0')
                {
                    addMessage("[Fout] Vul een gebruikersnaam in dumb dumb!");
                }
                else
                {
                    connected = client.connect(serverIp, static_cast<std::uint16_t>(port), username);
                    if (!connected)
                    {
                        addMessage("[Fout] Verbinden mislukt... womp womp");
                    }
                }
            }
            catch (...)
            {
                connected = false;
                addMessage("[Fout] Vul een geldige poort in - Moet je ook doen he");
            }
        }
    }
    else
    {
        ImGui::Text("Status- Verbonden");

        if (ImGui::Button("Verbreek verbinding :("))
        {
            client.disconnect();
            connected = false;
            addMessage("[Info] Verbinding verbroken");
        }
    }

    ImGui::Separator();
    ImGui::Text("Chatberichten");

    if (ImGui::BeginChild("Chatgeschiedenis", ImVec2(0, 300), true))
    {
        for (const std::string& message : messages)
        {
            ImGui::TextWrapped("%s", message.c_str());
        }
        if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
        {
            ImGui::SetScrollHereY(1.0f);
        }
    }
    ImGui::EndChild();
    ImGui::Spacing();

    ImGui::InputText("Bericht", messageInput, sizeof(messageInput));
    ImGui::SameLine();

    if (ImGui::Button("Verstuur") && connected)
    {
        if (messageInput[0] != '\0')
        {
            ChatMessage message;
            message.type = MessageType::Chat;
            message.text = messageInput;

            if (client.sendMessage(message))
            {
                messageInput[0] = '\0';
            }
            else
            {
                addMessage("[Fout] Bericht versturen mislukt... Womp Womp");
            }
        }
    }

    ImGui::End();
}
