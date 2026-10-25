#include "Opdracht4.hpp"

#include <imgui.h>
#include <cstdint>
#include <iostream>
#include <string>
#include <cctype>
#include <functional>

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
        onlineUsers.clear();
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

        case MessageType::UserList:
            onlineUsers = message.users;
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

    // Twee panelen naast elkaar.
    ImGui::BeginChild("OnlineGebruikers", ImVec2(220, 300), true);

    ImGui::Text("Online (%d)", static_cast<int>(onlineUsers.size()));
    ImGui::Separator();

    for (const std::string& user : onlineUsers)
    {
        // Bepaal een vaste kleur op basis van de gebruikersnaam.
        const std::size_t hash = std::hash<std::string>{}(user);
        const float hue = static_cast<float>(hash % 360) / 360.0f;
        const ImU32 avatarColor = ImColor::HSV(hue, 0.65f, 0.90f);

        const ImVec2 pos = ImGui::GetCursorScreenPos();
        ImDrawList* drawList = ImGui::GetWindowDrawList();

        // Teken de avatar.
        drawList->AddCircleFilled(
            ImVec2(pos.x + 10.0f, pos.y + 10.0f),
            9.0f,
            avatarColor
        );

        // Toon de eerste letter in de avatar.
        const char initial = user.empty()
            ? '?'
            : static_cast<char>(
                std::toupper(static_cast<unsigned char>(user[0]))
              );

        const std::string initialText(1, initial);

        drawList->AddText(
            ImVec2(pos.x + 6.0f, pos.y + 3.0f),
            IM_COL32(255, 255, 255, 255),
            initialText.c_str()
        );

        // Reserveer ruimte voor de avatar en zet de naam ernaast.
        ImGui::Dummy(ImVec2(22.0f, 20.0f));
        ImGui::SameLine();
        ImGui::TextUnformatted(user.c_str());
    }

    ImGui::EndChild();

    ImGui::SameLine();
    ImGui::BeginChild("Chatgeschiedenis", ImVec2(0, 300), true);
    ImGui::Text("Chatberichten");
    ImGui::Separator();

    for (const std::string& message : messages)
    {
        ImGui::TextWrapped("%s", message.c_str());
    }
    if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
    {
        ImGui::SetScrollHereY(1.0f);
    }

    ImGui::EndChild();
    ImGui::Spacing();
    ImGui::InputText("Bericht", messageInput, sizeof(messageInput));
    ImGui::InputText("Ontvanger (leeg = openbaar)", recipientInput, sizeof(recipientInput));

    const bool sendClicked = ImGui::Button("Verstuur");
    if (sendClicked && connected && messageInput[0] != '\0')
    {
        ChatMessage message;
        message.text = messageInput;

        if (recipientInput[0] != '\0')
        {
            message.type = MessageType::PrivateMessage;
            message.recipient = recipientInput;
        }
        else
        {
            message.type = MessageType::Chat;
        }

        if (client.sendMessage(message))
        {
            messageInput[0] = '\0';
        }
        else
        {
            addMessage("[Fout] Bericht versturen mislukt...");
        }
    }
    ImGui::End();
}
