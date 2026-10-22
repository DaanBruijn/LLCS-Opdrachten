#include "Opdracht4.hpp"
#include <imgui.h>

void Opdracht4::update()
{
    // - Voeg logica hier toe dumb dumb
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
            // - TCP Verbinding
        }
    }
    else
    {
        ImGui::Text("Status - Verbonden");
    }

    ImGui::Separator();
    ImGui::Text("Chatberichten verschijnen hier");

    ImGui::InputText("Bericht", messageInput, sizeof(messageInput));
    ImGui::SameLine();
    if (ImGui::Button("Verstuur"))
    {
        // - Send
    }

    ImGui::End();
}