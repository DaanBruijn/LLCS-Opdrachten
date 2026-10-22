add_executable(low-level_setup
        main.cpp
        Opdrachten/Opdracht4.cpp
        Opdrachten/Opdracht4.hpp

        Opdrachten/Opdracht4Client.cpp
        Opdrachten/Opdracht4Client.hpp

        Opdrachten/Opdracht4Server.cpp
        Opdrachten/Opdracht4Server.hpp

        Opdrachten/Opdracht4Protocol.hpp
)

target_link_libraries(low-level_setup
        PUBLIC
        ImGui-SFML::ImGui-SFML
        SFML::Network
)
