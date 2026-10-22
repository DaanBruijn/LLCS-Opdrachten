#include <SFML/Graphics.hpp>
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
    server.start(53000);

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
