add_executable(low-level_setup
        main.cpp
        Opdrachten/Opdracht2.cpp
        Opdrachten/Opdracht2.hpp
)

target_link_libraries(low-level_setup
        PUBLIC
        ImGui-SFML::ImGui-SFML
)