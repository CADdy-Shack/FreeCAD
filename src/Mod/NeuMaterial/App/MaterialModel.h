#pragma once

#include "PreCompiled.h"
#include <yaml-cpp/yaml.h>

namespace NeuMaterial::App {

// ---------------------------------------------------------------------------
// PropertyDefinition
//
// Metadata for a single property within a Model — type, units, description,
// valid range, and reference URL. Used by the validator and UI layer.
// ---------------------------------------------------------------------------

struct PropertyDefinition {
    std::string description;  // human-readable tooltip text
    std::string displayName;  // human-readable label e.g. "Young's Modulus"
    std::optional<double> maximum;  // nullopt if no upper bound
    std::optional<double> minimum;  // nullopt if no lower bound
    std::string name;         // UpperCamelCase key matching model file, e.g. "Density"
    std::string type;         // "Quantity", "Float", "String", "Color", "2DArray"
    std::string units;        // e.g. "kg/m^3", "GPa", "" if dimensionless
    std::string url;          // reference URL, may be empty
};

// ---------------------------------------------------------------------------
// InheritedModel
//
// Reference to a parent model included in a composite model's Inherited: list.
// ---------------------------------------------------------------------------

struct InheritedModel {
    std::string name;  // e.g. "Density"
    std::string uuid;  // e.g. "454661e5-265b-4320-8e6f-fcf6223ac3af"
};

// ---------------------------------------------------------------------------
// MaterialModel
//
// Represents a single loaded Model or AppearanceModel .yml file.
//
// Physical models use the top-level key 'Model:' and live under
// Resources/Models/ subdirectories (Mechanical/, Thermal/ etc.).
//
// Appearance models use the top-level key 'AppearanceModel:' and live
// under Resources/Models/Appearance/.
//
// Composite models have an Inherited list and an empty properties map.
// Granular models have no inherited entries and one or more properties.
// ---------------------------------------------------------------------------

class MaterialModel {
public:
    MaterialModel() = default;

    // ------------------------------------------------------------------
    // Model header accessors
    // ------------------------------------------------------------------

    const std::string& getName()        const { return name_;        }
    const std::string& getUuid()        const { return uuid_;        }
    const std::string& getUrl()         const { return url_;         }
    const std::string& getDescription() const { return description_; }
    const std::string& getDoi()         const { return doi_;         }

    // ------------------------------------------------------------------
    // Inheritance
    // ------------------------------------------------------------------

    const std::vector<InheritedModel>& inherited() const { return inherited_; }
    bool isAppearanceModel() const { return appearanceModel_; }
    bool isComposite()       const { return !inherited_.empty(); }

    // ------------------------------------------------------------------
    // Property definitions
    // ------------------------------------------------------------------

    const std::map<std::string, PropertyDefinition>& properties() const
        { return properties_; }

    /// Returns nullptr if the property is not defined in this model.
    const PropertyDefinition* property(const std::string& name) const;

    bool hasProperty(const std::string& name) const;

    // ------------------------------------------------------------------
    // Loading
    // ------------------------------------------------------------------

    /// Load a MaterialModel from a .yml file.
    /// Throws std::runtime_error on parse failure or missing required fields.
    static MaterialModel loadFromFile(const std::filesystem::path& path);

    /// Load a MaterialModel from a YAML string (for testing).
    static MaterialModel loadFromString(const std::string& yaml);

private:
    std::string name_;
    std::string uuid_;
    std::string url_;
    std::string description_;
    std::string doi_;

    bool                                      appearanceModel_ = false;
    std::vector<InheritedModel>               inherited_;
    std::map<std::string, PropertyDefinition> properties_;

    static MaterialModel parseNode(const YAML::Node& root,
                                   const std::string& source);
};

} // namespace NeuMaterial::App
