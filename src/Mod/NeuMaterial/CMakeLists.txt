cmake_minimum_required(VERSION 3.18)
project(NeuMaterial VERSION 0.1.0 LANGUAGES CXX)

# ---------------------------------------------------------------------------
# Global settings
# ---------------------------------------------------------------------------

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ---------------------------------------------------------------------------
# Find FreeCAD
# Assumes the workbench is built against an installed FreeCAD or its build tree.
# Set FreeCAD_DIR or FreeCAD_SOURCE_DIR on the cmake command line if needed.
# ---------------------------------------------------------------------------

find_package(FreeCAD REQUIRED)
find_package(Python3 COMPONENTS Interpreter Development REQUIRED)
find_package(yaml-cpp REQUIRED)

# ---------------------------------------------------------------------------
# Sub-directories
# ---------------------------------------------------------------------------

add_subdirectory(App)
add_subdirectory(Gui)

# ---------------------------------------------------------------------------
# Install Python packages (Init.py, InitGui.py, commands, ui)
# ---------------------------------------------------------------------------

install(FILES
    Init.py
    InitGui.py
    DESTINATION Mod/NeuMaterial
)

install(DIRECTORY
    Gui/commands
    Gui/ui
    Resources
    third_party
    DESTINATION Mod/NeuMaterial
)

install(DIRECTORY
    Gui/Resources
    DESTINATION Mod/NeuMaterial/Gui
)

# tools/ contains developer CLI scripts and is not installed
