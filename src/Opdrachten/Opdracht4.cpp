#include "Opdracht4.hpp"

#include <imgui.h>
#include <cstdint>
#include <iostream>
#include <string>
#include <cctype>
#include <functional>
#include <algorithm>
#include <chrono>

void Opdracht4::addMessage(const std::string &message) {
    messages.push_back(message);

    // - Limit messages in window
    constexpr std::size_t maxMessages = 200;

    if (messages.size() > maxMessages) {
        messages.erase(messages.begin());
    }

    std::cout << message << '\n';
}

void Opdracht4::update() {
    if (!client.isConnected()) {
        connected = false;
        onlineUsers.clear();
        typingUsers.clear();
        lastTypingState = false;
        return;
    }

    ChatMessage message;
    while (client.receiveMessage(message)) {
        switch (message.type) {
            case MessageType::Chat:
            case MessageType::Reply:
            case MessageType::PrivateMessage: {
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

            case MessageType::Typing: {
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
    for (auto it = typingUsers.begin(); it != typingUsers.end();) {
        if (it->second <= now)
            it = typingUsers.erase(it);
        else
            ++it;
    }

    connected = client.isConnected();
}


void Opdracht4::render()
{
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(16.0f, 14.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(9.0f, 7.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8.0f, 8.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 8.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 10.0f);

    ImGui::PushStyleColor(ImGuiCol_WindowBg,        ImVec4(0.075f, 0.085f, 0.11f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ChildBg,         ImVec4(0.10f, 0.115f, 0.15f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_Border,          ImVec4(0.20f, 0.23f, 0.29f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_FrameBg,         ImVec4(0.13f, 0.15f, 0.19f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered,  ImVec4(0.18f, 0.22f, 0.29f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_FrameBgActive,   ImVec4(0.20f, 0.25f, 0.34f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_Button,          ImVec4(0.20f, 0.40f, 0.72f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered,   ImVec4(0.25f, 0.48f, 0.82f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,    ImVec4(0.16f, 0.34f, 0.62f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_Header,          ImVec4(0.20f, 0.35f, 0.55f, 0.65f));
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered,   ImVec4(0.25f, 0.43f, 0.67f, 0.8f));
    ImGui::PushStyleColor(ImGuiCol_Separator,       ImVec4(0.23f, 0.27f, 0.34f, 1.0f));

    ImGui::SetNextWindowSize(ImVec2(1000.0f, 700.0f), ImGuiCond_FirstUseEver);
    ImGui::Begin("Chat Client");

    ImGui::TextColored(ImVec4(0.40f, 0.68f, 1.0f, 1.0f),"CHAT ROOM");
    ImGui::SameLine();
    ImGui::TextDisabled("/ EindOpdracht!");

    ImGui::SameLine(ImGui::GetWindowWidth() - 145.0f);

    if (connected)
        ImGui::TextColored(ImVec4(0.30f, 0.85f, 0.52f, 1.0f),"● Verbonden");
    else
        ImGui::TextColored(ImVec4(1.0f, 0.45f, 0.40f, 1.0f),"● Offline");

    ImGui::Separator();
    ImGui::Spacing();

    ImGui::Text("VERBINDING");
    ImGui::Spacing();

    ImGui::SetNextItemWidth(190.0f);
    ImGui::InputText("Server IP", serverIp, sizeof(serverIp));
    ImGui::SameLine();

    ImGui::SetNextItemWidth(90.0f);
    ImGui::InputText("Poort", serverPort, sizeof(serverPort));
    ImGui::SameLine();

    ImGui::SetNextItemWidth(170.0f);
    ImGui::InputText("Gebruikersnaam", username, sizeof(username));
    ImGui::SameLine();

    if (!connected)
    {
        if (ImGui::Button("Verbinden", ImVec2(120.0f, 0.0f)))
        {
            try
            {
                const int port = std::stoi(serverPort);

                if (port < 1 || port > 65535)
                {
                    addMessage("[Fout] Poort moet tussen 1 en 65535 liggen!");
                }
                else if (username[0] == '\0')
                {
                    addMessage("[Fout] Vul een gebruikersnaam in, dumb dumb!");
                }
                else
                {
                    connected = client.connect(serverIp, static_cast<std::uint16_t>(port),username);

                    if (!connected)
                    {
                        addMessage("[Fout] Verbinden mislukt... Womp Womp");
                    }
                }
            }
            catch (...)
            {
                connected = false;
                addMessage("[Fout] Vul een geldige poort in. Deze bestaat niet");
            }
        }
    }
    else
    {
        if (ImGui::Button("Verbreek", ImVec2(120.0f, 0.0f)))
        {
            client.disconnect();
            connected = false;
            addMessage("[Info] Verbinding verbroken, tot ziens!");
        }
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    const float availableHeight = ImGui::GetContentRegionAvail().y;
    const float chatHeight = availableHeight - 115.0f;

    ImGui::BeginChild("Online Users",ImVec2(220.0f, chatHeight), true);

    ImGui::Text("ONLINE");
    ImGui::SameLine();

    ImGui::TextColored(ImVec4(0.40f, 0.68f, 1.0f, 1.0f),"%d", static_cast<int>(onlineUsers.size()));

    ImGui::Separator();
    ImGui::Spacing();


    for (const std::string& user : onlineUsers)
    {
        const std::size_t hash = std::hash<std::string>{}(user);
        const float hue = static_cast<float>(hash % 360) / 360.0f;
        const ImU32 avatarColor = ImColor::HSV(hue, 0.65f, 0.90f);

        const float avatarSize = 24.0f;
        const float radius = avatarSize / 2.0f;
        const ImVec2 rowPos = ImGui::GetCursorScreenPos();
        ImDrawList* drawList = ImGui::GetWindowDrawList();

        const ImVec2 center(rowPos.x + radius,rowPos.y + radius);
        drawList->AddCircleFilled(center, radius, avatarColor);

        const char initial = user.empty() ? '?' : static_cast<char>(std::toupper(static_cast<unsigned char>(user[0])));
        const std::string initialText(1, initial);
        const ImVec2 textSize = ImGui::CalcTextSize(initialText.c_str());

        drawList->AddText(ImVec2(center.x - textSize.x / 2.0f, center.y - textSize.y / 2.0f), IM_COL32(255, 255, 255, 255),initialText.c_str());
        ImGui::SetCursorScreenPos(ImVec2(rowPos.x + avatarSize + 10.0f,center.y - ImGui::GetTextLineHeight() / 2.0f));

        ImGui::TextUnformatted(user.c_str());
        ImGui::SetCursorScreenPos(ImVec2(rowPos.x, rowPos.y + avatarSize + 6.0f));
    }

    ImGui::EndChild();
    ImGui::SameLine();

    ImGui::BeginChild("Chat", ImVec2(0.0f, chatHeight), true);

    ImGui::Text("Messages");
    ImGui::Separator();
    ImGui::Spacing();

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
            label = "[Prive] ";
        else if (entry.replyToId != 0)
            label = "[Antwoord] ";

        label += entry.username + "- " + entry.text;
        label += "##message" + std::to_string(entry.id);

        const bool canReply = !entry.isPrivate && entry.id != 0;

        if (canReply)
        {
            if (ImGui::Selectable(label.c_str(),replyToId == entry.id))
                replyToId = entry.id;
        }
        else
        {
            ImGui::TextWrapped("%s", label.c_str());
        }
    }

    if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
        ImGui::SetScrollHereY(1.0f);

    ImGui::EndChild();

    for (const auto& [typingUser, expiry] : typingUsers)
    {
        (void)expiry;

        ImGui::TextDisabled("%s is aan het typen...",typingUser.c_str());
    }

    if (replyToId != 0)
    {
        ImGui::TextColored(ImVec4(0.40f, 0.68f, 1.0f, 1.0f),"Antwoord op bericht #%u",replyToId);
        ImGui::SameLine();

        if (ImGui::SmallButton("Annuleer"))
            replyToId = 0;
    }

    ImGui::SetNextItemWidth(-105.0f);

    const bool enterPressed = ImGui::InputText("##BerichtInput", messageInput, sizeof(messageInput), ImGuiInputTextFlags_EnterReturnsTrue);
    ImGui::SameLine();

    const bool sendClicked = ImGui::Button("Verstuur", ImVec2(95.0f, 0.0f));

    const auto now = std::chrono::steady_clock::now();
    const bool typingNow = messageInput[0] != '\0';

    if (connected &&typingNow && (!lastTypingState || now - lastTypingSent >= std::chrono::seconds(1)))
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

    ImGui::SetNextItemWidth(260.0f);
    ImGui::InputText("Ontvanger (leeg = openbaar)", recipientInput, sizeof(recipientInput));

    if ((sendClicked || enterPressed) && connected && messageInput[0] != '\0')
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
            addMessage("[Fout] Bericht versturen mislukt... Womp Womp");
        }
    }

    ImGui::End();

    ImGui::PopStyleColor(12);
    ImGui::PopStyleVar(6);
}