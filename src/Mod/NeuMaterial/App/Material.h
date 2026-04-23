#pragma once

#include <string>
#include <optional>
#include <array>

namespace NeuMaterial::App {

// ---------------------------------------------------------------------------
// Mechanical properties
// ---------------------------------------------------------------------------
struct MechanicalProperties {
    std::optional<double> density;           // kg/m³
    std::optional<double> youngsModulus;     // GPa
    std::optional<double> poissonsRatio;     // dimensionless (0–0.5)
    std::optional<double> yieldStrength;     // MPa
    std::optional<double> ultimateStrength;  // MPa

    // bool operator==(const MechanicalProperties&) const = default;
    bool operator==(const MechanicalProperties& other) const {
        // Manually compare all relevant members
        return this->density == other.density &&
               this->youngsModulus == other.youngsModulus &&
               this->poissonsRatio == other.poissonsRatio &&
               this->yieldStrength == other.yieldStrength &&
               this->ultimateStrength == other.ultimateStrength;
    }};

// ---------------------------------------------------------------------------
// Thermal properties
// ---------------------------------------------------------------------------
struct ThermalProperties {
    std::optional<double> thermalConductivity; // W/m·K
    std::optional<double> thermalExpansion;    // µm/m·K
    std::optional<double> specificHeat;        // J/kg·K
    std::optional<double> meltingPoint;        // °C

    bool operator==(const ThermalProperties& other) const {
        return this->thermalConductivity == other.thermalConductivity &&
               this->thermalExpansion == other.thermalExpansion &&
               this->specificHeat == other.specificHeat &&
               this->meltingPoint == other.meltingPoint;
    };
};

// ---------------------------------------------------------------------------
// Electrical properties
// ---------------------------------------------------------------------------
struct ElectricalProperties {
    std::optional<double> electricalConductivity; // S/m
    std::optional<double> permittivity;           // F/m (absolute)
    std::optional<double> resistivity;            // Ω·m

    bool operator==(const ElectricalProperties& other) const
    {
        return this->electricalConductivity == other.electricalConductivity &&
               this->permittivity == other.permittivity &&
               this->resistivity == other.resistivity;
    };
};

// ---------------------------------------------------------------------------
// Appearance properties
// ---------------------------------------------------------------------------
struct AppearanceProperties {
    std::array<float, 4> color = {0.8f, 0.8f, 0.8f, 1.0f}; // RGBA 0–1
    std::string          texturePath;                        // path or empty
    float                roughness = 0.5f;                   // 0 (smooth) – 1 (rough)
    float                metallic  = 0.0f;                   // 0 (dielectric) – 1 (metal)
    float                opacity   = 1.0f;                   // 0 (transparent) – 1 (opaque)

    bool operator==(const AppearanceProperties& other) const
    {
        return this->color == other.color &&
               this->texturePath == other.texturePath &&
               this->roughness == other.roughness &&
               this->metallic == other.metallic &&
               this->opacity == other.opacity;
    };
};

// ---------------------------------------------------------------------------
// Material — top-level aggregate
// ---------------------------------------------------------------------------
class Material {
public:
    // Construction
    Material() = default;
    explicit Material(std::string name);

    // Identity
    const std::string& getId()          const { return id_;          }
    const std::string& getName()        const { return name_;        }
    const std::string& getDescription() const { return description_; }
    const std::string& getLibrary()     const { return library_;     }
    const std::string& getCategory()    const { return category_;    }
    bool               isReadOnly()     const { return readOnly_;    }

    void setName(std::string name);
    void setDescription(std::string description) { description_ = std::move(description); }
    void setLibrary(std::string library)         { library_     = std::move(library);     }
    void setCategory(std::string category)       { category_    = std::move(category);    }
    void setReadOnly(bool readOnly)              { readOnly_    = readOnly;               }

    // Property groups
    MechanicalProperties&       mechanical()       { return mechanical_; }
    const MechanicalProperties& mechanical() const { return mechanical_; }

    ThermalProperties&          thermal()          { return thermal_;    }
    const ThermalProperties&    thermal()    const { return thermal_;    }

    ElectricalProperties&       electrical()       { return electrical_; }
    const ElectricalProperties& electrical() const { return electrical_; }

    AppearanceProperties&       appearance()       { return appearance_; }
    const AppearanceProperties& appearance() const { return appearance_; }

    // Utility
    bool isValid()  const;
    bool operator==(const Material& other) const
    {
        return this->id_ == other.id_ &&
               this->name_ == other.name_ &&
               this->description_ == other.description_ &&
               this->library_ == other.library_ &&
               this->category_ == other.category_ &&
               this->readOnly_ == other.readOnly_ &&
               this->mechanical_ == other.mechanical_ &&
               this->thermal_ == other.thermal_ &&
               this->electrical_ == other.electrical_ &&
               this->appearance_ == other.appearance_;
    };

    /// Generate a filesystem-safe slug from the material name
    static std::string makeSlug(const std::string& name);

private:
    std::string id_;           // UUID or slug, set on construction/load
    std::string name_;
    std::string description_;
    std::string library_;      // owning library name
    std::string category_;     // e.g. "metals", "polymers"
    bool        readOnly_ = false;

    MechanicalProperties mechanical_;
    ThermalProperties    thermal_;
    ElectricalProperties electrical_;
    AppearanceProperties appearance_;

    void generateId();
};

} // namespace NeuMaterial::App
