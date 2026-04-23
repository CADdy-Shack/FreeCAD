#pragma once

#include "Material.h"

#include <filesystem>
#include <string>

namespace NeuMaterial::App {

// ---------------------------------------------------------------------------
// MaterialIO
//
// Stateless utility class for loading and saving Material objects to/from
// YAML files via yaml-cpp.
//
// File layout mirrors the schema defined in Material.h:
//
//   name: Steel 1020
//   description: Low-carbon structural steel
//   mechanical:
//     density: 7870
//     youngs_modulus: 205
//     ...
//   thermal:
//     ...
//   electrical:
//     ...
//   appearance:
//     color: [0.6, 0.6, 0.65, 1.0]
//     ...
// ---------------------------------------------------------------------------

class MaterialIO {
public:
    MaterialIO() = delete;

    /// Load a Material from a YAML file.
    /// The material's id is taken from the file if present, otherwise generated.
    /// Throws std::runtime_error on parse failure or missing required fields.
    static Material load(const std::filesystem::path& filePath);

    /// Load a Material from a YAML string (useful for testing).
    static Material loadFromString(const std::string& yaml);

    /// Save a Material to a YAML file.
    /// The parent directory must already exist.
    /// Throws std::runtime_error on a writing failure.
    static void save(const Material& material,
                     const std::filesystem::path& filePath);

    /// Serialize a Material to a YAML string.
    static std::string toString(const Material& material);

private:
    static Material parseNode(const YAML::Node& root);
    static YAML::Node toNode(const Material& material);
};

} // namespace NeuMaterial::App
