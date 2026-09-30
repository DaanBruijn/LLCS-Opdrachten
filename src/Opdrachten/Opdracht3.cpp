#include "Opdracht3.hpp"

#include <iostream>
#include <thread>

void ConcurrentInventory::addItem(const std::string& item)
{
    // - Vector lock zodat er maar een trhead items kan toevoegen
    std::lock_guard<std::mutex> lock(mutex);
    items.push_back(item);
}


std::size_t ConcurrentInventory::size() const
{
    std::lock_guard<std::mutex> lock(mutex);
    return items.size();
}


std::vector<std::string> ConcurrentInventory::getItems() const
{
    std::lock_guard<std::mutex> lock(mutex);
    return items;
}


void Opdracht3::update()
{
    if (!testDone)
    {
        runThreadTest();
        testDone = true;
    }
}


void Opdracht3::render()
{
    // - Temp ! Moet aangepast worden met ImgUI !
    std::cout << "ConcurrentInventory test" << std::endl;
    std::cout << "Verwacht aantal items: " << expectedItems << std::endl;
    std::cout << "Werkelijk aantal items: " << actualItems << std::endl;

    if (testPassed)
        std::cout << "TEST GESLAAGD: alle items zijn toegevoegd." << std::endl;
    else
        std::cout << "TEST MISLUKT." << std::endl;
}


void Opdracht3::runThreadTest()
{
    ConcurrentInventory inventory;

    constexpr std::size_t itemsPerThread = 1000;

    // - Thread 1
    std::thread thread1([&inventory]()
    {
        for (std::size_t i = 0; i < itemsPerThread; ++i)
        {
            inventory.addItem("Thread 1 - Item " + std::to_string(i));
        }
    });

    // - Thread 2
    std::thread thread2([&inventory]()
    {
        for (std::size_t i = 0; i < itemsPerThread; ++i)
        {
            inventory.addItem("Thread 2 - Item " + std::to_string(i));
        }
    });

    thread1.join();
    thread2.join();

    expectedItems = itemsPerThread * 2;
    actualItems = inventory.size();
    testPassed = actualItems == expectedItems;

    // - Temp ! Moet aangepast worden met ImgUI !
    std::cout << "====================================" << std::endl;
    std::cout << "ConcurrentInventory thread test" << std::endl;
    std::cout << "====================================" << std::endl;

    std::cout << "Thread 1 heeft "<< itemsPerThread<< " items toegevoegd." << std::endl;
    std::cout << "Thread 2 heeft "<< itemsPerThread<< " items toegevoegd." << std::endl;

    std::cout << "Verwacht: "<< expectedItems<< " items" << std::endl;
    std::cout << "Gevonden: "<< actualItems<< " items" << std::endl;

    if (testPassed)
        std::cout << "RESULTAAT: GESLAAGD" << std::endl;
    else
        std::cout << "RESULTAAT: MISLUKT" << std::endl;

    std::cout << "====================================" << std::endl;
}
