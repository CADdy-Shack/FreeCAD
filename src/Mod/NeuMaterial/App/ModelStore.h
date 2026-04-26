#pragma once

#include "PreCompiled.h"
#include "MaterialModel.h"

namespace NeuMaterial::App {

namespace fs = std::filesystem;

// ---------------------------------------------------------------------------
// ModelStore
//
// Discovers, loads, and caches all MaterialModel definitions from the
// Resources/Models/ directory tree. Loaded once at startup; read-only
// thereafter.
//
// Models are indexed by UUID (primary) and by name (secondary, for display).
// Composite models are fully resolved — their inherited property definitions
// are merged in so callers can ask "what properties does LinearElastic have?"
// without walking the inheritance chain themselves.
// ---------------------------------------------------------------------------

class ModelStore {
public:
    ModelStore()  = default;
    ~ModelStore() = default;

    ModelStore(const ModelStore&)            = delete;
    ModelStore& operator=(const ModelStore&) = delete;

    // ------------------------------------------------------------------
    // Initialization
    // ------------------------------------------------------------------

    /// Load all .yml files from modelsRootPath (Resources/Models/) recursively.
    /// Safe to call multiple times — clears and reloads each time.
    void initialise(const fs::path& modelsRootPath);

    // ------------------------------------------------------------------
    // Queries
    // ------------------------------------------------------------------

    /// Look up a model by UUID. Returns nullptr if not found.
    const MaterialModel* findByUuid(const std::string& uuid) const;

    /// Look up a model by name (case-sensitive). Returns nullptr if not found.
    const MaterialModel* findByName(const std::string& name) const;

    /// All loaded models, ordered by name.
    std::vector<const MaterialModel*> allModels() const;

    /// Returns true if the store has been successfully initialised.
    bool isReady() const { return ready_; }

    // ------------------------------------------------------------------
    // Resolved property lookup
    //
    // For composite models, merges inherited property definitions
    // so the caller gets a flat map of all properties the model covers.
    // ------------------------------------------------------------------

    /// Collect all PropertyDefinitions for a model UUID, resolving
    /// inheritance recursively. Returns an empty map if uuid is unknown.
    std::map<std::string, PropertyDefinition>
        resolvedProperties(const std::string& uuid) const;

    /// Given a material's list of model UUIDs, return every property
    /// definition across all of them (union, later entries win on conflict).
    std::map<std::string, PropertyDefinition>
        resolvedPropertiesForModels(const std::vector<std::string>& uuids) const;

private:
    std::map<std::string, std::shared_ptr<MaterialModel>> byUuid_;
    std::map<std::string, std::string>                    nameToUuid_;
    bool                                                   ready_ = false;

    void loadFile(const fs::path& path);
    void resolveInto(const std::string& uuid,
                     std::map<std::string, PropertyDefinition>& out,
                     std::vector<std::string>& visited) const;
};

} // namespace NeuMaterial::App
