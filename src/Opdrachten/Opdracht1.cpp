#include "Opdracht1.hpp"

#include <imgui.h>
#include <random>
#include <chrono>
#include <iostream>


Opdracht1::Opdracht1(std::size_t width, std::size_t height)
    : width(width),
      height(height),
      grid(height, std::vector<bool>(width, false)),
      nextGrid(height, std::vector<bool>(width, false))
{
    randomizeGrid();
}


void Opdracht1::update()
{
    drawImGui();

    if (!running)
        return;

    updateTimer += 1.0f / 60.0f;

    if (updateTimer >= updateInterval)
    {
        updateTimer = 0.0f;
        updateGrid();
    }
}


int Opdracht1::countNeighbours(std::size_t x, std::size_t y) const
{
    int neighbours = 0;

    // - We controlleren  de 8 mogelijke buren rondom een cel


    for (int offsetY = -1; offsetY <= 1; ++offsetY)
    {
        for (int offsetX = -1; offsetX <= 1; ++offsetX)
        {
            // - De huidige cel word niet gecontroleerd
            if (offsetX == 0 && offsetY == 0)
                continue;

            const int neighbourX = static_cast<int>(x) + offsetX;
            const int neighbourY = static_cast<int>(y) + offsetY;

            // - Buiten het grid kan niet
            if (neighbourX < 0 || neighbourY < 0 || neighbourX >= static_cast<int>(width) || neighbourY >= static_cast<int>(height))
                continue;

            if (grid[neighbourY][neighbourX])
                ++neighbours;
        }
    }
    return neighbours;
}


void Opdracht1::updateGrid()
{
    const auto start = std::chrono::high_resolution_clock::now();

    for (auto rowIterator = grid.begin(); rowIterator != grid.end(); ++rowIterator)
    {
        const std::size_t y = std::distance(grid.begin(), rowIterator);

        for (auto cellIterator = rowIterator->begin(); cellIterator != rowIterator->end(); ++cellIterator)
        {
            const std::size_t x = std::distance(rowIterator->begin(), cellIterator);
            const int neighbours = countNeighbours(x, y);

            if (*cellIterator)
                nextGrid[y][x] = neighbours == 2 || neighbours == 3;
            else
                nextGrid[y][x] = neighbours == 3;
        }
    }

    grid.swap(nextGrid);
    ++generation;

    const auto end = std::chrono::high_resolution_clock::now();
    const std::chrono::duration<float, std::milli> elapsed = end - start;

    std::cout << "Generatie " << generation << ": " << elapsed.count() << " ms" << std::endl;
}


void Opdracht1::render(sf::RenderWindow& window) {
    const float cellWidth = static_cast<float>(window.getSize().x) / static_cast<float>(width);
    const float cellHeight = static_cast<float>(window.getSize().y) / static_cast<float>(height);

    sf::RectangleShape cell;
    cell.setFillColor(sf::Color::White);

    for (auto rowIterator = grid.begin(); rowIterator != grid.end(); ++rowIterator)
    {
        const std::size_t y = std::distance(grid.begin(), rowIterator);

        for(auto cellIterator = rowIterator->begin(); cellIterator != rowIterator->end(); ++cellIterator)
        {
            const std::size_t x = std::distance(rowIterator->begin(), cellIterator);

            // - Alleen de levende cellen maken
            if (!*cellIterator)
                continue;


            cell.setPosition({static_cast<float>(x) * cellWidth,static_cast<float>(y) * cellHeight});
            cell.setSize({cellWidth - 1.0f,cellHeight - 1.0f});
            window.draw(cell);
        }
    }
}

void Opdracht1::reset()
{
    for (auto& row : grid)
    {
        for (std::size_t i = 0; i < row.size(); ++i)
        {
            row[i] = false;
        }
    }

    for (auto& row : nextGrid)
    {
        for (std::size_t i = 0; i < row.size(); ++i)
        {
            row[i] = false;
        }
    }

    generation = 0;
    updateTimer = 0.0f;

    randomizeGrid();
}


void Opdracht1::randomizeGrid()
{
    std::random_device randomDevice;
    std::mt19937 generator(randomDevice());

    // - ~25% van de grid is een pixel
    std::uniform_int_distribution<int> distribution(0, 3);

    for (auto& row : grid)
    {
        for (std::size_t i = 0; i < row.size(); ++i)
        {
            row[i] = distribution(generator) == 0;
        }
    }
}


void Opdracht1::drawImGui()
{
    ImGui::Begin("Conway's Game of Life");

    ImGui::Text("Grid: %zu x %zu",width,height);
    ImGui::Text("Generatie: %d",generation);

    ImGui::Separator();

    // - Pause en Start knoppies
    if (running)
    {
        if (ImGui::Button("Pause"))
            running = false;
    }
    else
    {
        if (ImGui::Button("Start"))
            running = true;
    }


    ImGui::SameLine();
    // - Reset Knoppie
    if (ImGui::Button("Reset"))
        reset();

    ImGui::Separator();
    // - Slider voor snelheid van de simulatie
    ImGui::SliderFloat("Update interval",&updateInterval,0.01f,1.0f,"%.2f sec");
    ImGui::Text("Updates per seconde: %.1f",1.0f / updateInterval);

    ImGui::Separator();

    ImGui::Text("Conway's regels:");
    ImGui::BulletText("minder dan 2 buren - cel sterft");
    ImGui::BulletText("2 of 3 buren - cel blijft leven");
    ImGui::BulletText("Meer dan 3 buren - cell sterft");
    ImGui::BulletText("Precies 3 buren - cel wordt geboren");
    ImGui::End();
}