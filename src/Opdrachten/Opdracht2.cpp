#include "Opdracht2.hpp"

#include <imgui.h>
#include <algorithm>
#include <chrono>
#include <cmath>
#include <iostream>

Ball::Ball(float x,float y,float radius,sf::Color color,float vx,float vy)
{
    shape.setRadius(radius);
    shape.setOrigin({radius, radius});
    shape.setPosition({x, y});
    shape.setFillColor(color);

    velocity = {vx, vy};
}

std::size_t BallGame::CellHash::operator()(const Cell& cell) const
{
    const std::size_t x =static_cast<std::size_t>(cell.x * 73856093);
    const std::size_t y =static_cast<std::size_t>(cell.y * 19349663);

    return x ^ y;
}
// - Constructor
BallGame::BallGame() : gen(rd()), velDist(-200.0f, 200.0f), colorDist(50, 255)
{
    createBalls(5000);
}
// - Ball generation
void BallGame::createBalls(std::size_t amount)
{
    balls.clear();
    balls.reserve(amount);

    std::uniform_real_distribution<float> xDist(5.0f, 1275.0f);
    std::uniform_real_distribution<float> yDist(5.0f, 715.0f);

    constexpr float radius = 2.5f;

    for (std::size_t i = 0; i < amount; ++i)
    {
        const float x = xDist(gen);
        const float y = yDist(gen);
        const float vx = velDist(gen);
        const float vy = velDist(gen);

        sf::Color color(static_cast<std::uint8_t>(colorDist(gen)),static_cast<std::uint8_t>(colorDist(gen)),static_cast<std::uint8_t>(colorDist(gen)));
        balls.emplace_back(x,y,radius,color,vx,vy);
    }
}

BallGame::Cell BallGame::getCell(const sf::Vector2f& position) const
{
    return
    {
        static_cast<int>(std::floor(position.x / cellSize)),
        static_cast<int>(std::floor(position.y / cellSize))
    };
}

void BallGame::rebuildSpatialHash()
{
    spatialHash.clear();

    for (std::size_t i = 0; i < balls.size(); ++i)
    {
        const sf::Vector2f position =balls[i].shape.getPosition();
        const Cell cell = getCell(position);

        spatialHash[cell].push_back(i);
    }
}

void BallGame::update(const sf::Vector2u& windowSize, float deltaTime)
{
    const auto frameStart = std::chrono::steady_clock::now();

    collisionTests = 0;
    collisionsResolved = 0;

    updatePositions(deltaTime, windowSize);

    // - Spatial Hash opbouwen
    rebuildSpatialHash();
    // - Meet de Spatial Hash-methode
    const auto spatialStart = std::chrono::steady_clock::now();
    handleBallCollisions();

    const auto spatialEnd = std::chrono::steady_clock::now();
    collisionTimeMs =std::chrono::duration<float, std::milli>(spatialEnd - spatialStart).count();

    const auto frameEnd = std::chrono::steady_clock::now();
    frameTimeMs =std::chrono::duration<float, std::milli>(frameEnd - frameStart).count();

    drawImGui();
}

void BallGame::updatePositions(float deltaTime, const sf::Vector2u& windowSize)
{
    for (auto& ball : balls)
    {
        ball.shape.move(ball.velocity * deltaTime);
    }

    handleWallCollisions(windowSize);
}

// - Handle wall collision
void BallGame::handleWallCollisions(const sf::Vector2u& windowSize)
{
    for (auto& ball : balls)
    {
        sf::Vector2f position = ball.shape.getPosition();
        const float radius = ball.shape.getRadius();

        if (position.x - radius <= 0.0f)
        {
            position.x = radius;
            ball.velocity.x = std::abs(ball.velocity.x);
        }
        else if (position.x + radius >= static_cast<float>(windowSize.x))
        {
            position.x = static_cast<float>(windowSize.x) - radius;
            ball.velocity.x = -std::abs(ball.velocity.x);
        }

        if (position.y - radius <= 0.0f)
        {
            position.y = radius;
            ball.velocity.y = std::abs(ball.velocity.y);
        }
        else if (position.y + radius >= static_cast<float>(windowSize.y))
        {
            position.y = static_cast<float>(windowSize.y) - radius;
            ball.velocity.y = -std::abs(ball.velocity.y);
        }

        ball.shape.setPosition(position);
    }
}


void BallGame::handleBallCollisions()
{
    for (std::size_t i = 0; i < balls.size(); ++i)
    {
        for (int offsetY = -1; offsetY <= 1; ++offsetY)
        {
            for (int offsetX = -1; offsetX <= 1; ++offsetX)
            {
                const sf::Vector2f pos1 =
                    balls[i].shape.getPosition();

                const Cell currentCell = getCell(pos1);

                const Cell neighbourCell{
                    currentCell.x + offsetX,
                    currentCell.y + offsetY
                };

                const auto iterator = spatialHash.find(neighbourCell);

                if (iterator == spatialHash.end())
                    continue;

                for (const std::size_t j : iterator->second)
                {
                    if (j <= i)
                        continue;

                    Ball& ball1 = balls[i];
                    Ball& ball2 = balls[j];

                    const sf::Vector2f pos1 = ball1.shape.getPosition();
                    const sf::Vector2f pos2 = ball2.shape.getPosition();

                    const sf::Vector2f delta = pos2 - pos1;

                    const float distanceSquared = delta.x * delta.x + delta.y * delta.y;

                    const float minDistance = ball1.shape.getRadius() + ball2.shape.getRadius();
                    ++collisionTests;

                    if (distanceSquared >= minDistance * minDistance)
                        continue;

                    if (distanceSquared <= 0.000001f)
                        continue;

                    const float distance = std::sqrt(distanceSquared);
                    const sf::Vector2f normal = delta / distance;

                    const float overlap = minDistance - distance;

                    // - Correct the overlap regardless of velocity
                    constexpr float correctionPercent = 0.8f;
                    const sf::Vector2f correction = normal * (overlap * correctionPercent * 0.5f);

                    ball1.shape.setPosition(pos1 - correction);
                    ball2.shape.setPosition(pos2 + correction);

                    // - Only apply an impulse if the balls approach
                    const sf::Vector2f relativeVelocity = ball2.velocity - ball1.velocity;
                    const float velocityAlongNormal = relativeVelocity.x * normal.x + relativeVelocity.y * normal.y;

                    if (velocityAlongNormal >= 0.0f)
                        continue;

                    constexpr float restitution = 1.0f;
                    const float impulseMagnitude = -(1.0f + restitution) * velocityAlongNormal * 0.5f;
                    const sf::Vector2f impulse = impulseMagnitude * normal;

                    ball1.velocity -= impulse;
                    ball2.velocity += impulse;

                    ++collisionsResolved;
                }
            }
        }
    }
}

void BallGame::handleBallCollisionsBruteForce()
{
    bruteForceTests = 0;

    for (std::size_t i = 0; i < balls.size(); ++i)
    {
        for (std::size_t j = i + 1; j < balls.size(); ++j)
        {
            ++bruteForceTests;

            Ball& ball1 = balls[i];
            Ball& ball2 = balls[j];

            const sf::Vector2f pos1 = ball1.shape.getPosition();
            const sf::Vector2f pos2 = ball2.shape.getPosition();

            const sf::Vector2f delta = pos2 - pos1;

            const float distanceSquared =
                delta.x * delta.x + delta.y * delta.y;

            const float minDistance =
                ball1.shape.getRadius() + ball2.shape.getRadius();

            if (distanceSquared >= minDistance * minDistance)
                continue;

            if (distanceSquared <= 0.000001f)
                continue;

            const float distance = std::sqrt(distanceSquared);
            const sf::Vector2f normal = delta / distance;
            const float overlap = minDistance - distance;

            const sf::Vector2f separation = normal * (overlap * 0.5f);

            ball1.shape.setPosition(pos1 - separation);
            ball2.shape.setPosition(pos2 + separation);

            const sf::Vector2f relativeVelocity = ball2.velocity - ball1.velocity;
            const float velocityAlongNormal = relativeVelocity.x * normal.x + relativeVelocity.y * normal.y;

            if (velocityAlongNormal >= 0.0f)
                continue;

            constexpr float restitution = 1.0f;
            const float impulseMagnitude = -(1.0f + restitution) * velocityAlongNormal * 0.5f;

            const sf::Vector2f impulse = impulseMagnitude * normal;

            ball1.velocity -= impulse;
            ball2.velocity += impulse;

            ++collisionsResolved;
        }
    }
}

void BallGame::render(sf::RenderWindow& window) const
{
    for (const auto& ball : balls)
    {
        window.draw(ball.shape);
    }
}


void BallGame::drawImGui()
{
    ImGui::Begin("Spatial Hashing");

    const std::size_t n = balls.size();
    const std::size_t bruteForceCount = n > 1 ? n * (n - 1) / 2 : 0;

    ImGui::Text("Ballen: %zu", n);
    ImGui::Text("Celgrootte: %.1f px", cellSize);
    ImGui::Separator();

    ImGui::Text("Brute Force tests: %zu", bruteForceCount);
    ImGui::Text("Spatial Hash tests: %zu", collisionTests);
    ImGui::Text("Botsingen opgelost: %zu", collisionsResolved);

    if (bruteForceCount > 0)
    {
        const double reduction =100.0 * ( 1.0 - static_cast<double>(collisionTests)/static_cast<double>(bruteForceCount));
        ImGui::Text("Reductie controles: %.1f%%", reduction);
    }

    ImGui::Separator();
    ImGui::Text("Spatial Hash collision time: %.3f ms", collisionTimeMs);
    ImGui::Text("Physics frame time: %.3f ms", frameTimeMs);

    if (frameTimeMs > 0.0f)
    {
        ImGui::Text("Physics FPS: %.1f",1000.0f/frameTimeMs);
    }
    ImGui::Separator();

    ImGui::Text("Aantal hash cells: %zu", spatialHash.size());
    ImGui::End();
}
