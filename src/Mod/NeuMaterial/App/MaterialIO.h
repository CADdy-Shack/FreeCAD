#pragma once

#include "PreCompiled.h"
#include "Material.h"
#include <yaml-cpp/yaml.h>

namespace NeuMaterial::App {

// ---------------------------------------------------------------------------
// MaterialIO
//
// Stateless utility class for loading and saving Material objects to/from
// YAML files via yaml-cpp.
//
// File format (version 2.0):
//
//   ---
//   General:
//     Author: 'Michael Cummings'
//     Description: 'Low-carbon structural steel, hot-rolled'
//     License: 'CC-BY-4.0'
//     Name: 'Steel 1020'
//     Tags: ['hot-rolled', 'low-carbon', 'steel']
//     URL: 'https://en.wikipedia.org/wiki/Steel'
//     UUID: '7b4e1a2f-3c5d-4e6f-8a9b-0c1d2e3f4a5b'
//     Version: '2.0'
//   Models:
//     LinearElastic:
//       Density: '7870 kg/m^3'
//       UUID: '9cdda8be-b8fb-4c62-95c2-74e852d4afae'
//       YoungsModulus: '205 GPa'
//       ...
//     Thermal:
//       ...
//   ...
//
// All keys are UpperCamelCase for compatibility with FreeCAD's material system.
// All sections and keys within sections are written sorted a-z.
// Property values are strings of the form 'value unit' e.g. '7870 kg/m^3'.
// Category is NOT stored in the file — it is derived from the file path
// by MaterialStore after loading.
// ---------------------------------------------------------------------------

class MaterialIO {
public:
    MaterialIO() = delete;

    /// Load a Material from a .yaml file.
    /// Throws std::runtime_error on parse failure or missing required fields.
    static Material load(const std::filesystem::path& filePath);

    /// Load a Material from a YAML string (useful for testing).
    static Material loadFromString(const std::string& yaml);

    /// Save a Material to a .yaml file.
    /// Writes General: then Models: sorted a-z, with --- header and ... footer.
    /// The parent directory must already exist.
    /// Throws std::runtime_error on write failure.
    static void save(const Material& material,
                     const std::filesystem::path& filePath);

    /// Serialise a Material to a YAML string.
    static std::string toString(const Material& material);

private:
    static Material parseNode(const YAML::Node& root,
                              const std::string& source);

    // Parsing helpers
    static void parseGeneral   (const YAML::Node& general,
                                Material& mat,
                                const std::string& source);
    static void parseModels          (const YAML::Node& models,
                                    Material& mat);
    static void parseAppearanceModels(const YAML::Node& models,
                                    Material& mat);
    static void parseLinearElastic   (const YAML::Node& node, Material& mat);
    static void parseThermal         (const YAML::Node& node, Material& mat);
    static void parseElectrical      (const YAML::Node& node, Material& mat);
    static void parseBasicRendering  (const YAML::Node& node, Material& mat);

    // Serialization helpers
    static void emitAppearanceModels (YAML::Emitter& out, const Material& mat);
    static void emitBasicRendering   (YAML::Emitter& out, const Material& mat);
    static void emitElectrical       (YAML::Emitter& out, const Material& mat);
    static void emitGeneral          (YAML::Emitter& out, const Material& mat);
    static void emitLinearElastic    (YAML::Emitter& out, const Material& mat);
    static void emitModels           (YAML::Emitter& out, const Material& mat);
    static void emitThermal          (YAML::Emitter& out, const Material& mat);

    // Value parsing — extracts numeric value from 'value unit' string
    static std::optional<double> parseQuantity(const YAML::Node& node,
                                               const char* key);
    static std::optional<float>  parseFloat   (const YAML::Node& node,
                                               const char* key);
    // Color: '(r, g, b)' or '(r, g, b, a)' → array<float,4>
    static bool parseColor(const std::string& str,
                           std::array<float, 4>& out);
};

} // namespace NeuMaterial::App
