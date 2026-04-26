#pragma once

#include "PreCompiled.h"

namespace NeuMaterial::App {

// ---------------------------------------------------------------------------
// AppearanceProperties
// ---------------------------------------------------------------------------
struct AppearanceProperties {
    std::array<float, 4> color       = {0.8f, 0.8f, 0.8f, 1.0f}; // RGBA 0–1
    float                metallic    = 0.0f;  // 0 (dielectric) – 1 (metal)
    float                opacity     = 1.0f;  // 0 (transparent) – 1 (opaque)
    float                roughness   = 0.5f;  // 0 (smooth) – 1 (rough)
    std::string          texturePath;         // path or empty

    bool operator==(const AppearanceProperties& o) const {
        return color       == o.color
            && metallic    == o.metallic
            && opacity     == o.opacity
            && roughness   == o.roughness
            && texturePath == o.texturePath;
    }
    bool operator!=(const AppearanceProperties& o) const { return !(*this == o); }
};

// ---------------------------------------------------------------------------
// ElectricalProperties
// ---------------------------------------------------------------------------
struct ElectricalProperties {
    std::optional<double> electricalConductivity; // S/m
    std::optional<double> permittivity;           // F/m (absolute)
    std::optional<double> resistivity;            // Ω·m

    bool operator==(const ElectricalProperties& o) const {
        return electricalConductivity == o.electricalConductivity
            && permittivity           == o.permittivity
            && resistivity            == o.resistivity;
    }
    bool operator!=(const ElectricalProperties& o) const { return !(*this == o); }
};

// ---------------------------------------------------------------------------
// MechanicalProperties
// ---------------------------------------------------------------------------
struct MechanicalProperties {
    std::optional<double> density;           // kg/m³
    std::optional<double> poissonsRatio;     // dimensionless (0–0.5)
    std::optional<double> ultimateStrength;  // MPa
    std::optional<double> yieldStrength;     // MPa
    std::optional<double> youngsModulus;     // GPa

    bool operator==(const MechanicalProperties& o) const {
        return density          == o.density
            && poissonsRatio    == o.poissonsRatio
            && ultimateStrength == o.ultimateStrength
            && yieldStrength    == o.yieldStrength
            && youngsModulus    == o.youngsModulus;
    }
    bool operator!=(const MechanicalProperties& o) const { return !(*this == o); }
};

// ---------------------------------------------------------------------------
// ThermalProperties
// ---------------------------------------------------------------------------
struct ThermalProperties {
    std::optional<double> meltingPoint;        // °C
    std::optional<double> specificHeat;        // J/kg·K
    std::optional<double> thermalConductivity; // W/m·K
    std::optional<double> thermalExpansion;    // µm/m·K

    bool operator==(const ThermalProperties& o) const {
        return meltingPoint        == o.meltingPoint
            && specificHeat        == o.specificHeat
            && thermalConductivity == o.thermalConductivity
            && thermalExpansion    == o.thermalExpansion;
    }
    bool operator!=(const ThermalProperties& o) const { return !(*this == o); }
};

// ---------------------------------------------------------------------------
// Material
//
// Top-level aggregate representing a single material.
//
// Identity comes from the General: block in the YAML file — uuid, name,
// description, author, license, url, version, and tags are all loaded
// from there.
//
// modelUuids_ lists the Models: declared in the file, linking this material
// to its property schema definitions in the ModelStore.
//
// Library, category, and read-only state are store-level concerns set by
// MaterialStore after loading; they are not persisted in the YAML file.
// ---------------------------------------------------------------------------
class Material {
public:
    Material() = default;
    explicit Material(std::string name);

    // ------------------------------------------------------------------
    // General metadata (from General: block, written/read sorted a-z)
    // ------------------------------------------------------------------

    const std::string&              getAuthor()      const { return author_;      }
    const std::string&              getDescription() const { return description_; }
    const std::string&              getLicense()     const { return license_;     }
    const std::string&              getName()        const { return name_;        }
    const std::vector<std::string>& getTags()        const { return tags_;        }
    const std::string&              getUrl()         const { return url_;         }
    const std::string&              getUuid()        const { return uuid_;        }
    const std::string&              getVersion()     const { return version_;     }

    void addTag(std::string tag)                 { tags_.push_back(std::move(tag));  }
    void setAuthor(std::string author)           { author_      = std::move(author); }
    void setDescription(std::string desc)        { description_ = std::move(desc);   }
    void setLicense(std::string license)         { license_     = std::move(license);}
    void setName(std::string name)               { name_        = std::move(name);   }
    void setTags(std::vector<std::string> tags)  { tags_        = std::move(tags);   }
    void setUrl(std::string url)                 { url_         = std::move(url);    }
    void setUuid(std::string uuid)               { uuid_        = std::move(uuid);   }
    void setVersion(std::string version)         { version_     = std::move(version);}

    // ------------------------------------------------------------------
    // Store-level metadata (set by MaterialStore, not persisted in YAML)
    // ------------------------------------------------------------------

    const std::string& getCategory() const { return category_; }
    const std::string& getLibrary()  const { return library_;  }
    bool               isReadOnly()  const { return readOnly_;  }

    void setCategory(std::string category) { category_ = std::move(category); }
    void setLibrary(std::string library)   { library_  = std::move(library);  }
    void setReadOnly(bool readOnly)        { readOnly_  = readOnly;            }

    // ------------------------------------------------------------------
    // Model references
    // ------------------------------------------------------------------

    const std::vector<std::string>& modelUuids() const { return modelUuids_; }
    void addModelUuid(std::string uuid) { modelUuids_.push_back(std::move(uuid)); }
    void clearModelUuids()              { modelUuids_.clear(); }

    // ------------------------------------------------------------------
    // Property groups (nested structs — authoritative data)
    // ------------------------------------------------------------------

    AppearanceProperties&       appearance()       { return appearance_; }
    const AppearanceProperties& appearance() const { return appearance_; }

    ElectricalProperties&       electrical()       { return electrical_; }
    const ElectricalProperties& electrical() const { return electrical_; }

    MechanicalProperties&       mechanical()       { return mechanical_; }
    const MechanicalProperties& mechanical() const { return mechanical_; }

    ThermalProperties&          thermal()          { return thermal_;    }
    const ThermalProperties&    thermal()    const { return thermal_;    }

    // ------------------------------------------------------------------
    // Utility
    // ------------------------------------------------------------------

    /// Returns true if the material has a non-empty name and uuid.
    bool isValid() const;

    bool operator==(const Material& o) const {
        return author_      == o.author_
            && appearance_  == o.appearance_
            && category_    == o.category_
            && description_ == o.description_
            && electrical_  == o.electrical_
            && library_     == o.library_
            && license_     == o.license_
            && mechanical_  == o.mechanical_
            && modelUuids_  == o.modelUuids_
            && name_        == o.name_
            && tags_        == o.tags_
            && thermal_     == o.thermal_
            && url_         == o.url_
            && uuid_        == o.uuid_
            && version_     == o.version_;
    }
    bool operator!=(const Material& o) const { return !(*this == o); }

    /// Generate a filesystem-safe slug from a material name.
    /// Used by MaterialStore to derive a save path for new user materials.
    static std::string makeSlug(const std::string& name);

private:
    // General: block (a-z)
    std::string              author_;
    std::string              description_;
    std::string              license_;
    std::string              name_;
    std::vector<std::string> tags_;
    std::string              url_;
    std::string              uuid_;
    std::string              version_  = "2.0";

    // Store-level (not persisted, a-z)
    std::string category_;
    std::string library_;
    bool        readOnly_ = false;

    // Models: block
    std::vector<std::string> modelUuids_;

    // Property data (a-z)
    AppearanceProperties appearance_;
    ElectricalProperties electrical_;
    MechanicalProperties mechanical_;
    ThermalProperties    thermal_;
};

} // namespace NeuMaterial::App
