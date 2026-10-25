#include <SFML/Graphics.hpp>
#include <iostream>
#include "imgui-SFML.h"

#include "Opdrachten/Opdracht4.hpp"
#include "Opdrachten/Opdracht4Server.hpp"

int main()
{
    sf::RenderWindow window(sf::VideoMode({1280, 720}),"Concurrent Inventory");

    window.setFramerateLimit(60);
    window.setVerticalSyncEnabled(true);

    if (!ImGui::SFML::Init(window))
        return -1;

    sf::Clock deltaClock;
    Opdracht4 opdracht;

    Opdracht4Server server;
    const bool isServer = server.start(53000);

    if (!isServer)
    {
        std::cout << "Deze instantie draait alleen als client - Dubbel poort bug fixed\n";
    }

    while (window.isOpen())
    {
        while (const std::optional event = window.pollEvent())
        {
            ImGui::SFML::ProcessEvent(window, *event);

            if (event->is<sf::Event::Closed>())
                window.close();
        }

        const sf::Time deltaTime = deltaClock.restart();

        ImGui::SFML::Update(window, deltaTime);

        opdracht.update();
        window.clear(sf::Color::Black);
        opdracht.render();

        ImGui::SFML::Render(window);
        window.display();
    }

    server.stop();

    ImGui::SFML::Shutdown();

    return 0;
}
