#include "Opdracht1.hpp"

Opdracht1::Opdracht1(std::size_t width, std::size_t height)
    : width(width),
      height(height),
      grid(height, std::vector<bool>(width, false)),
      nextGrid(height, std::vector<bool>(width, false))
{
    // - Kleiner grid  *test*
    grid[1][2] = true;
    grid[2][3] = true;
    grid[3][1] = true;
    grid[3][2] = true;
    grid[3][3] = true;
}

void Opdracht1::update()
{
    updateGrid();
}

int Opdracht1::countNeighbours(std::size_t x, std::size_t y) const
{
    int neighbours = 0;

    for (int offsetY = -1; offsetY <= 1; ++offsetY)
    {
        for (int offsetX = -1; offsetX <= 1; ++offsetX)
        {
            if (offsetX == 0 && offsetY == 0)
                continue;

            const int neighbourX = static_cast<int>(x) + offsetX;
            const int neighbourY = static_cast<int>(y) + offsetY;

            if (neighbourX < 0 ||neighbourY < 0 ||neighbourX >= static_cast<int>(width) ||neighbourY >= static_cast<int>(height))
                continue;

            if (grid[neighbourY][neighbourX])
                ++neighbours;
        }
    }
    return neighbours;
}

void Opdracht1::updateGrid()
{
    for (std::size_t y = 0; y < height; ++y)
    {
        for (std::size_t x = 0; x < width; ++x)
        {
            const int neighbours = countNeighbours(x, y);

            if (grid[y][x])
                nextGrid[y][x] = (neighbours == 2 || neighbours == 3);
            else
                nextGrid[y][x] = (neighbours == 3);
        }
    }

    grid.swap(nextGrid);
}

void Opdracht1::render(sf::RenderWindow& window)
{
    const float cellWidth = static_cast<float>(window.getSize().x) /static_cast<float>(width);
    const float cellHeight = static_cast<float>(window.getSize().y) /static_cast<float>(height);

    sf::RectangleShape cell;
    cell.setFillColor(sf::Color::White);

    for (std::size_t y = 0; y < height; ++y)
    {
        for (std::size_t x = 0; x < width; ++x)
        {
            if (!grid[y][x])
                continue;

            cell.setPosition({static_cast<float>(x) * cellWidth,static_cast<float>(y) * cellHeight});
            cell.setSize({cellWidth - 1.0f,cellHeight - 1.0f});
            window.draw(cell);
        }
    }
}
