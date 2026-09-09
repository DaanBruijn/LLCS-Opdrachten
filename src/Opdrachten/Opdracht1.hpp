#pragma once

#include <SFML/Graphics.hpp>
#include <vector>
#include <cstddef>

class Opdracht1
{
public:
    Opdracht1(std::size_t width = 30, std::size_t height = 30);
    void update();
    void render(sf::RenderWindow& window);

private:
    std::size_t width;
    std::size_t height;
    std::vector<std::vector<bool>> grid;
    std::vector<std::vector<bool>> nextGrid;

    bool running = false;
    int generation = 0;
    float updateInterval = 0.1f;
    float updateTimer = 0.0f;

    int countNeighbours(std::size_t x, std::size_t y) const;

    void updateGrid();
    void reset();
    void drawImGui();
    void randomizeGrid();
};
