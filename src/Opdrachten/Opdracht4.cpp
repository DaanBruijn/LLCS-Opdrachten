#include "Opdracht4.hpp"

#include <imgui.h>
#include <cstdint>
#include <iostream>
#include <string>
#include <cctype>
#include <functional>
#include <algorithm>
#include <chrono>

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
        typingUsers.clear();
        lastTypingState = false;
        return;
    }

    ChatMessage message;
    while (client.receiveMessage(message))
    {
        switch (message.type)
        {
        case MessageType::Chat:
        case MessageType::Reply:
        case MessageType::PrivateMessage:
        {
            ChatEntry entry;
            entry.id = message.messageId;
            entry.replyToId = message.replyToId;
            entry.username = message.username;
            entry.text = message.text;
            entry.isPrivate = message.type == MessageType::PrivateMessage;

            chatEntries.push_back(std::move(entry));

            constexpr std::size_t maxChatEntries = 200;

            if (chatEntries.size() > maxChatEntries)
                chatEntries.erase(chatEntries.begin());

            typingUsers.erase(message.username);
            break;
        }

        case MessageType::ServerEvent:
            addMessage("[Server] " + message.text);
            break;

        case MessageType::Error:
            addMessage("[Error] " + message.text);
            break;

        case MessageType::UserList:
            onlineUsers = message.users;
            break;

        case MessageType::Typing:
        {
            if (message.username == username)
                break;

            if (message.isTyping)
                typingUsers[message.username] = std::chrono::steady_clock::now() + std::chrono::seconds(3);
            else
                typingUsers.erase(message.username);

            break;
        }

        default:
            break;
        }
    }

    const auto now = std::chrono::steady_clock::now();
    for (auto it = typingUsers.begin(); it != typingUsers.end();)
    {
        if (it->second <= now)
            it = typingUsers.erase(it);
        else
            ++it;
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
    ImGui::BeginChild("OnlineGebruikers", ImVec2(220, 300), true);

    ImGui::Text("Online (%d)", static_cast<int>(onlineUsers.size()));
    ImGui::Separator();

    for (const std::string& user : onlineUsers)
    {
        const std::size_t hash = std::hash<std::string>{}(user);
        const float hue = static_cast<float>(hash % 360) / 360.0f;
        const ImU32 avatarColor = ImColor::HSV(hue, 0.65f, 0.90f);

        const ImVec2 pos = ImGui::GetCursorScreenPos();
        ImDrawList* drawList = ImGui::GetWindowDrawList();

        drawList->AddCircleFilled(ImVec2(pos.x + 10.0f, pos.y + 10.0f),9.0f,avatarColor);
        const char initial = user.empty()? '?': static_cast<char>(std::toupper(static_cast<unsigned char>(user[0])));

        const std::string initialText(1, initial);

        drawList->AddText(ImVec2(pos.x + 6.0f, pos.y + 3.0f),IM_COL32(255, 255, 255, 255),initialText.c_str());

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
    for (const ChatEntry& entry : chatEntries)
    {
        if (entry.replyToId != 0)
        {
            const auto original = std::find_if(chatEntries.begin(), chatEntries.end(), [&entry](const ChatEntry& candidate)
            {
                return candidate.id == entry.replyToId;
            });

            if (original != chatEntries.end())
                ImGui::TextDisabled("Antwoord op %s: %s",original->username.c_str(), original->text.c_str());
            else
                ImGui::TextDisabled("Antwoord op bericht #%u", entry.replyToId);
        }
        std::string label;

        if (entry.isPrivate)
            label = "[Privé] ";
        else if (entry.replyToId != 0)
            label = "[Antwoord] ";

        label += entry.username + ": " + entry.text;
        label += "##message" + std::to_string(entry.id);

        const bool canReply = !entry.isPrivate && entry.id != 0;

        if (canReply)
        {
            if (ImGui::Selectable(label.c_str(), replyToId == entry.id))
                replyToId = entry.id;
        }
        else
            ImGui::TextWrapped("%s", label.c_str());
    }

    if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
        ImGui::SetScrollHereY(1.0f);

    ImGui::EndChild();


    ImGui::Spacing();
    if (replyToId != 0)
    {
        ImGui::Text("Antwoord op bericht #%u", replyToId);
        ImGui::SameLine();

        if (ImGui::SmallButton("Annuleer antwoord"))
            replyToId = 0;
    }

    ImGui::InputText("Bericht", messageInput, sizeof(messageInput));

    const auto now = std::chrono::steady_clock::now();
    const bool typingNow = messageInput[0] != '\0';

    if (connected && typingNow && (!lastTypingState || now - lastTypingSent >= std::chrono::seconds(1)))
    {
        ChatMessage typing;
        typing.type = MessageType::Typing;
        typing.isTyping = true;

        if (client.sendMessage(typing))
        {
            lastTypingState = true;
            lastTypingSent = now;
        }
    }
    else if (connected && !typingNow && lastTypingState)
    {
        ChatMessage typing;
        typing.type = MessageType::Typing;
        typing.isTyping = false;

        client.sendMessage(typing);
        lastTypingState = false;
    }

    ImGui::InputText("Ontvanger (leeg = openbaar)", recipientInput, sizeof(recipientInput));

    // Toon welke gebruikers aan het typen zijn.
    for (const auto& [typingUser, expiry] : typingUsers)
    {
        (void)expiry;
        ImGui::TextDisabled("%s is aan het typen...", typingUser.c_str());
    }

    if (ImGui::Button("Verstuur") && connected && messageInput[0] != '\0')
    {
        ChatMessage outgoing;
        outgoing.text = messageInput;

        if (recipientInput[0] != '\0')
        {
            outgoing.type = MessageType::PrivateMessage;
            outgoing.recipient = recipientInput;
        }
        else if (replyToId != 0)
        {
            outgoing.type = MessageType::Reply;
            outgoing.replyToId = replyToId;
    }
    else
    {
        outgoing.type = MessageType::Chat;
    }

        if (client.sendMessage(outgoing))
        {
            messageInput[0] = '\0';
            recipientInput[0] = '\0';
            replyToId = 0;
        }
        else
        {
            addMessage("[Fout] Bericht versturen mislukt");
        }
    }

    ImGui::End();
}
