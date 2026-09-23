#pragma once

#include <SFML/Graphics.hpp>
#include <cstddef>
#include <random>
#include <unordered_map>
#include <vector>

struct Ball
{
    sf::CircleShape shape;
    sf::Vector2f velocity;

    Ball(float x,float y,float radius,sf::Color color,float vx,float vy);
};

class BallGame
{
public:
    BallGame();
    void update(const sf::Vector2u& windowSize, float deltaTime);
    void render(sf::RenderWindow& window) const;

private:

    struct Cell
    {
        int x;
        int y;

        bool operator==(const Cell& other) const
        {
            return x == other.x && y == other.y;
        }
    };

    struct CellHash
    {
        std::size_t operator()(const Cell& cell) const;
    };

    using SpatialHash =std::unordered_map<Cell, std::vector<std::size_t>, CellHash>;
    SpatialHash spatialHash;

    std::vector<Ball> balls; // - hehe Balls
    float cellSize = 10.0f;

    std::random_device rd;
    std::mt19937 gen;
    std::uniform_real_distribution<float> velDist;
    std::uniform_int_distribution<int> colorDist;
    std::size_t collisionTests = 0;
    std::size_t collisionsResolved = 0;

    float collisionTimeMs = 0.0f;
    float frameTimeMs = 0.0f;

    void createBalls(std::size_t amount);
    void rebuildSpatialHash();
    void updatePositions(float deltaTime,const sf::Vector2u& windowSize);
    void handleWallCollisions(const sf::Vector2u& windowSize);
    void handleBallCollisions();
    void resolveCollision(Ball& ball1, Ball& ball2);

    Cell getCell(const sf::Vector2f& position) const;

    void drawImGui();
};