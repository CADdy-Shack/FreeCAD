#pragma once

#include "Material.h"

#include <filesystem>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace NeuMaterial::App {

namespace fs = std::filesystem;

// ---------------------------------------------------------------------------
// Library descriptor
// ---------------------------------------------------------------------------

struct Library {
    std::string name;       // display name
    fs::path    rootPath;   // absolute path to the library directory
    bool        readOnly;   // true for the bundled library
};

// ---------------------------------------------------------------------------
// MaterialStore
//
// Central registry for all libraries and materials.
// One instance lives for the lifetime of the workbench session.
//
// Responsibilities:
//   - Discover and register libraries (built-in + user)
//   - Load / reload materials from disk on demand
//   - CRUD operations for user-library materials
//   - Provide a flat or filtered view of all materials
// ---------------------------------------------------------------------------

class MaterialStore {
public:
    MaterialStore();
    ~MaterialStore() = default;

    // Non-copyable, movable
    MaterialStore(const MaterialStore&)            = delete;
    MaterialStore& operator=(const MaterialStore&) = delete;
    MaterialStore(MaterialStore&&)                 = default;
    MaterialStore& operator=(MaterialStore&&)      = default;

    // ------------------------------------------------------------------
    // Initialisation
    // ------------------------------------------------------------------

    /// Call once at workbench startup.
    /// @param resourcesPath     Absolute path to the workbench Resources/ directory.
    ///                          Standard materials are loaded from Resources/Material/Standard/
    ///                          and appearance materials from Resources/Material/Appearance/.
    /// @param userLibrariesRoot Parent dir for user libraries.
    ///                          Defaults to ~/.local/share/FreeCAD/NeuMaterial/libraries/
    ///                          (Linux/macOS) or %APPDATA%\FreeCAD\NeuMaterial\libraries\ (Windows).
    void initialise(const fs::path& resourcesPath,
                    const fs::path& userLibrariesRoot = {});

    // ------------------------------------------------------------------
    // Library management
    // ------------------------------------------------------------------

    const std::vector<Library>& libraries() const { return libraries_; }

    /// Create a new (empty) user library.
    /// Returns false if a library with that name already exists.
    bool createUserLibrary(const std::string& name);

    /// Remove a user library and all its materials from disk.
    /// The built-in library cannot be removed.
    bool removeUserLibrary(const std::string& name);

    // ------------------------------------------------------------------
    // Material queries
    // ------------------------------------------------------------------

    /// All materials across all libraries.
    std::vector<std::shared_ptr<Material>> allMaterials() const;

    /// Materials belonging to a specific library.
    std::vector<std::shared_ptr<Material>>
        materialsInLibrary(const std::string& libraryName) const;

    /// Lookup by id. Returns nullptr if not found.
    std::shared_ptr<Material> findById(const std::string& id) const;

    /// Lookup by name (case-insensitive). Returns first match or nullptr.
    std::shared_ptr<Material> findByName(const std::string& name) const;

    // ------------------------------------------------------------------
    // CRUD — only permitted on user-library materials
    // ------------------------------------------------------------------

    /// Add a new material to the named user library and persist it.
    /// Throws std::invalid_argument if the library is read-only or missing.
    std::shared_ptr<Material> addMaterial(Material material,
                                          const std::string& libraryName);

    /// Persist changes to an existing user-library material.
    /// Throws std::runtime_error if the material is read-only.
    void updateMaterial(const Material& material);

    /// Delete a user-library material from memory and disk.
    /// Throws std::runtime_error if the material is read-only.
    void deleteMaterial(const std::string& id);

    /// Duplicate any material (including read-only) into a user library.
    std::shared_ptr<Material> duplicateMaterial(const std::string& sourceId,
                                                const std::string& targetLibrary,
                                                const std::string& newName = {});

    // ------------------------------------------------------------------
    // Persistence helpers
    // ------------------------------------------------------------------

    /// Reload all materials from disk (e.g. after external edit).
    void reload();

private:
    std::vector<Library>                                     libraries_;
    std::map<std::string, std::shared_ptr<Material>>         materialsById_;

    fs::path userLibrariesRoot_;

    void discoverLibrary(const Library& lib);
    fs::path materialFilePath(const Material& material) const;
    Library* findLibrary(const std::string& name);
    const Library* findLibrary(const std::string& name) const;
};

// ---------------------------------------------------------------------------
// Global singleton accessor
// Use this to reach the store from Python bindings, Gui commands, etc.
// ---------------------------------------------------------------------------

/// Returns the process-wide MaterialStore instance.
/// Initialized once by NeuMaterialApp::initializeApp().
MaterialStore& instance();   // defined in NeuMaterialApp.cpp

} // namespace NeuMaterial::App
