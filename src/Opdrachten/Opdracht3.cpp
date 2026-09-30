#include "Opdracht3.hpp"

#include <imgui.h>
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
    // - Test een keer runnen
    if (!testDone)
    {
        runThreadTest();
        testDone = true;
    }
}

void Opdracht3::render()
{
    ImGui::Begin("Concurrent Inventory Test");
    ImGui::Text("ConcurrentInventory thread test");
    ImGui::Separator();

    ImGui::Text("Thread 1: %zu items", itemsPerThread);
    ImGui::Text("Thread 2: %zu items", itemsPerThread);

    ImGui::Spacing();
    ImGui::Text("Verwacht aantal items: %zu", expectedItems);
    ImGui::Text("Werkelijk aantal items: %zu", actualItems);

    ImGui::Spacing();

    if (testPassed)
        ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.2f, 1.0f),"RESULTAAT: GESLAAGD");
    else
        ImGui::TextColored(ImVec4(1.0f, 0.2f, 0.2f, 1.0f),"RESULTAAT: MISLUKT");
    ImGui::Separator();


    // - Test opnieuw runnen
    if (ImGui::Button("Test opnieuw uitvoeren"))
        testDone = false;

    ImGui::End();
}

void Opdracht3::runThreadTest()
{
    ConcurrentInventory inventory;

    // - Thread 1
    std::thread thread1([&inventory]()
    {
        for (std::size_t i = 0; i < itemsPerThread; ++i)
        {
            inventory.addItem(
                "Thread 1 - Item " + std::to_string(i)
            );
        }
    });

    // - Thread 2
    std::thread thread2([&inventory]()
    {
        for (std::size_t i = 0; i < itemsPerThread; ++i)
        {
            inventory.addItem(
                "Thread 2 - Item " + std::to_string(i)
            );
        }
    });

    thread1.join();
    thread2.join();

    expectedItems = itemsPerThread * 2;
    actualItems = inventory.size();

    testPassed = actualItems == expectedItems;
}
