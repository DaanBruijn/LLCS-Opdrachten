#pragma once
#include <vector>
#include <string>

#include <mutex>
#include <cstddef>

class ConcurrentInventory
{
public:
    void addItem(const std::string& item);

    std::size_t size() const;
    std::vector<std::string> getItems() const;

private:
    std::vector<std::string> items;mutable
    std::mutex mutex;
};

class Opdracht3
{
public:
    void update();
    void render();

private:
    bool testDone = false;
    bool testPassed = false;
    std::size_t expectedItems = 0;
    std::size_t actualItems = 0;

    void runThreadTest();
};