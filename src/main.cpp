#include "imgui-SFML.h"
#include <SFML/Graphics.hpp>

#include "Opdrachten/Opdracht1.hpp"

int main()
{
    sf::RenderWindow window;

    window.create(sf::VideoMode({1280, 720}),"Conway's Game of Life");
    window.setFramerateLimit(60);
    window.setVerticalSyncEnabled(true);

    if (!ImGui::SFML::Init(window))
        return -1;

    sf::Clock deltaClock;
    Opdracht1 game(30, 30);

    while (window.isOpen())
    {
        while (const std::optional event = window.pollEvent())
        {
            ImGui::SFML::ProcessEvent(window, *event);

            if (event->is<sf::Event::Closed>())
                window.close();
        }

        ImGui::SFML::Update(window,deltaClock.restart());

        game.update();
        window.clear(sf::Color::Black);
        game.render(window);
        ImGui::SFML::Render(window);
        window.display();
    }

    ImGui::SFML::Shutdown();

    return 0;
}
