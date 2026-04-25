#pragma once

#include "PreCompiled.h"
#include "Material.h"

namespace NeuMaterial::App
{

namespace fs = std::filesystem;

// ---------------------------------------------------------------------------
// Library
// ---------------------------------------------------------------------------

struct Library
{
    std::string name;   // display name
    bool readOnly;      // true for built-in libraries
    fs::path rootPath;  // absolute path to the library directory
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
//
// UUID indexes the materials. Category and library membership are
// derived at load time from the file path — not stored in the YAML.
// ---------------------------------------------------------------------------

class MaterialStore
{
public:
    MaterialStore() = default;
    ~MaterialStore() = default;

    MaterialStore(const MaterialStore&) = delete;
    MaterialStore& operator=(const MaterialStore&) = delete;
    MaterialStore(MaterialStore&&) = default;
    MaterialStore& operator=(MaterialStore&&) = default;

    // ------------------------------------------------------------------
    // Initialization
    // ------------------------------------------------------------------

    /// Call once at workbench startup, after ModelStore is initialized.
    /// @param resourcesPath  Absolute path to the workbench Resources/ dir.
    ///   Standard materials:  Resources/Material/Standard/
    ///   Appearance materials: Resources/Material/Appearance/
    /// @param userLibrariesRoot  Parent dir for user libraries.
    ///   Linux:   ~/.local/share/FreeCAD/NeuMaterial/libraries/
    ///   macOS:   ~/Library/Application Support/FreeCAD/NeuMaterial/libraries/
    ///   Windows: %APPDATA%\FreeCAD\NeuMaterial\libraries\.
    void initialize(const fs::path& resourcesPath, const fs::path& userLibrariesRoot = {});

    // ------------------------------------------------------------------
    // Library management
    // ------------------------------------------------------------------

    const std::vector<Library>& libraries() const
    {
        return libraries_;
    }

    /// Create a new empty user library. Returns false if name already exists.
    bool createUserLibrary(const std::string& name);

    /// Remove a user library and all its materials from the disk.
    /// Built-in libraries cannot be removed. Returns false on failure.
    bool removeUserLibrary(const std::string& name);

    // ------------------------------------------------------------------
    // Material queries
    // ------------------------------------------------------------------

    /// All materials across all libraries.
    std::vector<std::shared_ptr<Material>> allMaterials() const;

    /// Materials belonging to a specific library.
    std::vector<std::shared_ptr<Material>> materialsInLibrary(const std::string& libraryName) const;

    /// Lookup by UUID. Returns nullptr if not found.
    std::shared_ptr<Material> findByUuid(const std::string& uuid) const;

    /// Lookup by name (case-insensitive). Returns first match or nullptr.
    std::shared_ptr<Material> findByName(const std::string& name) const;

    // ------------------------------------------------------------------
    // CRUD — only permitted on user-library materials
    // ------------------------------------------------------------------

    /// Add a new material to the named user library and persist it.
    /// The material must have a non-empty UUID and Name before calling.
    /// Throws std::invalid_argument if the library is read-only or missing.
    std::shared_ptr<Material> addMaterial(Material material, const std::string& libraryName);

    /// Persist changes to an existing user-library material.
    /// Throws std::runtime_error if the material is read-only or not found.
    void updateMaterial(const Material& material);

    /// Delete a user-library material from memory and disk.
    /// Throws std::runtime_error if the material is read-only.
    void deleteMaterial(const std::string& uuid);

    /// Duplicate any material (including read-only) into a user library.
    /// Assigns a new UUID to the copy. newName defaults to "<name> (copy)".
    std::shared_ptr<Material> duplicateMaterial(
        const std::string& sourceUuid,
        const std::string& targetLibrary,
        const std::string& newName = {}
    );

    // ------------------------------------------------------------------
    // Persistence
    // ------------------------------------------------------------------

    /// Reload all materials from disk (e.g. after external edit).
    void reload();

private:
    std::vector<Library> libraries_;
    std::map<std::string, std::shared_ptr<Material>> byUuid_;
    fs::path userLibrariesRoot_;

    void discoverLibrary(const Library& lib);
    fs::path materialFilePath(const Material& material) const;
    Library* findLibrary(const std::string& name);
    const Library* findLibrary(const std::string& name) const;

    static std::string generateUuid();
};

}  // namespace NeuMaterial::App
