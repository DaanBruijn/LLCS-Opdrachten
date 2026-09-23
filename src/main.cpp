#include <SFML/Graphics.hpp>
#include "imgui-SFML.h"

#include "Opdrachten/Opdracht2.hpp"

int main()
{
    sf::RenderWindow window(sf::VideoMode({1280, 720}),"Spatial Hashing - Ball Collision");

    window.setFramerateLimit(60);
    window.setVerticalSyncEnabled(true);

    if (!ImGui::SFML::Init(window))
        return -1;

    sf::Clock deltaClock;
    BallGame game;

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
        game.update(window.getSize(),deltaTime.asSeconds());
        window.clear(sf::Color::Black);
        game.render(window);
        ImGui::SFML::Render(window);
        window.display();
    }

    ImGui::SFML::Shutdown();

    return 0;
}