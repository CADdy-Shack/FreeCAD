#pragma once

// Standard library headers used widely across the App layer.
// Listed here so the compiler can precompile them once.

#include <algorithm>
#include <array>
#include <filesystem>
#include <map>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

// yaml-cpp — included globally so every translation unit can parse/emit YAML
// without repeating the include. Make sure yaml-cpp is found by CMake.
#include <yaml-cpp/yaml.h>
