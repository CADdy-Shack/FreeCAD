cmake_minimum_required(VERSION 3.18)

find_package(Qt6 COMPONENTS Core Widgets REQUIRED)

# ---------------------------------------------------------------------------
# Qt resource file (.qrc)
# Compiles icons and images in Resources/ into the shared library so FreeCAD
# can load them via QIcon(":/NeuMaterial/icons/...") at runtime.
# ---------------------------------------------------------------------------

set(NeuMaterialGui_QRC  ${CMAKE_CURRENT_SOURCE_DIR}/Resources/NeuMaterial.qrc)

# Generate the .qrc file listing every file under Resources/icons and
# Resources/images at configure time, then compile it with Qt6.
# (If you prefer to maintain the .qrc by hand, remove the block below and
# keep a hand-written Resources/NeuMaterial.qrc.)

file(GLOB_RECURSE _icon_files
        RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
        Resources/icons/*
        Resources/images/*
)

set(_qrc_content "<!DOCTYPE RCC>\n<RCC version=\"1.0\">\n  <qresource prefix=\"/NeuMaterial\">\n")
foreach(_f IN LISTS _icon_files)
    string(APPEND _qrc_content "    <file>${_f}</file>\n")
endforeach()
string(APPEND _qrc_content "  </qresource>\n</RCC>\n")

file(WRITE ${NeuMaterialGui_QRC} "${_qrc_content}")

qt6_add_resources(NeuMaterialGui_QRC_SOURCES ${NeuMaterialGui_QRC})

# ---------------------------------------------------------------------------
# Source files
# ---------------------------------------------------------------------------

set(NeuMaterialGui_SOURCES
    NeuMaterialGui.cpp
    Workbench.cpp
    ${NeuMaterialGui_QRC_SOURCES}
)

set(NeuMaterialGui_HEADERS
    PreCompiled.h
    NeuMaterialGui.h
    Workbench.h
)

# ---------------------------------------------------------------------------
# Shared library
# ---------------------------------------------------------------------------

add_library(NeuMaterialGui SHARED
    ${NeuMaterialGui_SOURCES}
    ${NeuMaterialGui_HEADERS}
)

target_precompile_headers(NeuMaterialGui PRIVATE PreCompiled.h)

target_include_directories(NeuMaterialGui
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE
        ${CMAKE_SOURCE_DIR}/App         # NeuMaterialApp headers
        ${FreeCAD_SOURCE_DIR}/src
        ${PYTHON_INCLUDE_DIRS}
)

target_link_libraries(NeuMaterialGui
    PUBLIC
        NeuMaterialApp
        FreeCADGui
        Qt6::Core
        Qt6::Widgets
    PRIVATE
        ${PYTHON_LIBRARIES}
)

target_compile_features(NeuMaterialGui PUBLIC cxx_std_17)

if(MSVC)
    target_compile_options(NeuMaterialGui PRIVATE /W4 /WX-)
else()
    target_compile_options(NeuMaterialGui PRIVATE -Wall -Wextra -Wpedantic)
endif()

# ---------------------------------------------------------------------------
# Install translations (Qt .qm files, built separately via lupdate/lrelease)
# ---------------------------------------------------------------------------

install(DIRECTORY Resources/translations/
        DESTINATION Mod/NeuMaterial/Gui/Resources/translations
        FILES_MATCHING PATTERN "*.qm"
)

# UI (.ui) files are used at runtime by uic / Python -- install them as-is
install(DIRECTORY Resources/ui/
        DESTINATION Mod/NeuMaterial/Gui/Resources/ui
        FILES_MATCHING PATTERN "*.ui"
)

# ---------------------------------------------------------------------------
# Install shared library
# ---------------------------------------------------------------------------

install(TARGETS NeuMaterialGui
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
