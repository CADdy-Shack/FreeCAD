cmake_minimum_required(VERSION 3.18)

# ---------------------------------------------------------------------------
# Source files
# ---------------------------------------------------------------------------

set(NeuMaterialApp_SOURCES
        NeuMaterialApp.cpp
        Material.cpp
        MaterialStore.cpp
        MaterialIO.cpp
        MaterialValidator.cpp
)

set(NeuMaterialApp_HEADERS
        PreCompiled.h
        NeuMaterialApp.h
        Material.h
        MaterialStore.h
        MaterialIO.h
        MaterialValidator.h
)


# ---------------------------------------------------------------------------
# Generate Python bindings from .pyi descriptors
# Uses FreeCAD's generate_from_py CMake function
# ---------------------------------------------------------------------------

set(NeuMaterialApp_PYI_SRCS
        Material.pyi
        MaterialStore.pyi
        ModelStore.pyi
)

generate_from_py(NeuMaterialApp_PYI_SRCS NeuMaterialApp_PY_SRCS)

# ---------------------------------------------------------------------------
# Shared library target
# ---------------------------------------------------------------------------

add_library(NeuMaterialApp SHARED
        ${NeuMaterialApp_SOURCES}
        ${NeuMaterialApp_HEADERS}
        ${NeuMaterialApp_PY_SRCS}
)

# Precompiled header (CMake 3.16+)
target_precompile_headers(NeuMaterialApp PRIVATE PreCompiled.h)

# ---------------------------------------------------------------------------
# Include directories
# ---------------------------------------------------------------------------

target_include_directories(NeuMaterialApp
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        PRIVATE
        ${FreeCAD_SOURCE_DIR}/src        # FreeCAD base headers
        ${PYTHON_INCLUDE_DIRS}
)

# ---------------------------------------------------------------------------
# Dependencies
# ---------------------------------------------------------------------------

# yaml-cpp — required for YAML I/O
find_package(yaml-cpp REQUIRED)

target_link_libraries(NeuMaterialApp
        PUBLIC
        yaml-cpp::yaml-cpp
        FreeCADBase              # App::Application, Base::Console, etc.
        PRIVATE
        ${PYTHON_LIBRARIES}
)

# ---------------------------------------------------------------------------
# Compile options
# ---------------------------------------------------------------------------

target_compile_features(NeuMaterialApp PUBLIC cxx_std_17)

if(MSVC)
    target_compile_options(NeuMaterialApp PRIVATE /W4 /WX-)
else()
    target_compile_options(NeuMaterialApp PRIVATE -Wall -Wextra -Wpedantic)
endif()

# ---------------------------------------------------------------------------
# Install
# ---------------------------------------------------------------------------

install(TARGETS NeuMaterialApp
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}   # Windows DLL
)

install(FILES ${NeuMaterialApp_HEADERS}
        DESTINATION include/NeuMaterial/App
)
