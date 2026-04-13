# Game library - ImGui-based interface, depends on CoreEngine

add_library(GameLib STATIC
    game.cpp
    inputManager.cpp
)

# Public headers for game
target_include_directories(GameLib PUBLIC
    ${CMAKE_SOURCE_DIR}                 # Root directory for #include <game/...>
    ${IMGUI_BACKENDS_DIR}/..            # ImGui include directory
)

# Link against core engine and ImGui
target_link_libraries(GameLib PUBLIC
    VozelEngine                          # Public: exposes core to targets linking GameLib
    imgui::imgui                        # ImGui for UI
)

# Set C++20 standard
set_property(TARGET GameLib PROPERTY CXX_STANDARD 20)
set_property(TARGET GameLib PROPERTY CXX_STANDARD_REQUIRED ON)

# Optional: Add a folder for Visual Studio solution
set_property(TARGET GameLib PROPERTY FOLDER "Game")