add_executable(low-level_setup
        main.cpp
        Opdrachten/Opdracht3.cpp
        Opdrachten/Opdracht3.hpp
)

target_link_libraries(low-level_setup
        PUBLIC
        ImGui-SFML::ImGui-SFML
)