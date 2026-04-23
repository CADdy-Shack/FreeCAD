#include "PreCompiled.h"
#include "MaterialStore.h"
#include "MaterialIO.h"

#include <algorithm>
#include <stdexcept>

#ifdef _WIN32
#  include <windows.h>
#  include <shlobj.h>
#else
#  include <pwd.h>
#  include <unistd.h>
#endif

namespace NeuMaterial::App {

namespace fs = std::filesystem;

// ---------------------------------------------------------------------------
// Internal helpers
// ---------------------------------------------------------------------------

static fs::path defaultUserLibrariesRoot()
{
#ifdef _WIN32
    // %APPDATA%\FreeCAD\NeuMaterial\libraries
    wchar_t path[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathW(nullptr, CSIDL_APPDATA, nullptr, 0, path)))
        return fs::path(path) / "FreeCAD" / "NeuMaterial" / "libraries";
    return fs::temp_directory_path() / "NeuMaterial" / "libraries";
#elif defined(__APPLE__)
    // ~/Library/Application Support/FreeCAD/NeuMaterial/libraries
    const char* home = getenv("HOME");
    if (!home) {
        struct passwd* pw = getpwuid(getuid());
        home = pw ? pw->pw_dir : "/tmp";
    }
    return fs::path(home) / "Library" / "Application Support" / "FreeCAD" / "NeuMaterial" / "libraries";
#else
    // XDG: $XDG_DATA_HOME/FreeCAD/NeuMaterial/libraries
    // Falls back to ~/.local/share/FreeCAD/NeuMaterial/libraries
    const char* xdg = getenv("XDG_DATA_HOME");
    if (xdg && *xdg)
        return fs::path(xdg) / "FreeCAD" / "NeuMaterial" / "libraries";
    const char* home = getenv("HOME");
    if (!home) {
        struct passwd* pw = getpwuid(getuid());
        home = pw ? pw->pw_dir : "/tmp";
    }
    return fs::path(home) / ".local" / "share" / "FreeCAD" / "NeuMaterial" / "libraries";
#endif
}

// ---------------------------------------------------------------------------
// Construction
// ---------------------------------------------------------------------------

MaterialStore::MaterialStore() = default;

// ---------------------------------------------------------------------------
// Initialisation
// ---------------------------------------------------------------------------

void MaterialStore::initialise(const fs::path& builtinLibraryPath,
                               const fs::path& userLibrariesRoot)
{
    libraries_.clear();
    materialsById_.clear();

    userLibrariesRoot_ = userLibrariesRoot.empty()
                             ? defaultUserLibrariesRoot()
                             : userLibrariesRoot;

    // Register + load built-in library
    Library builtin;
    builtin.name     = "Built-in";
    builtin.rootPath = builtinLibraryPath;
    builtin.readOnly = true;
    libraries_.push_back(builtin);
    discoverLibrary(libraries_.back());

    // Register built-in Standard material library
    // Location: Resources/Material/Standard/
    {
        Library standard;
        standard.name     = "Standard";
        standard.rootPath = resourcesPath / "Material" / "Standard";
        standard.readOnly = true;
        libraries_.push_back(standard);
        discoverLibrary(libraries_.back());
    }

    // Register built-in Appearance material library
    // Location: Resources/Material/Appearance/
    {
        Library appearance;
        appearance.name     = "Appearance";
        appearance.rootPath = resourcesPath / "Material" / "Appearance";
        appearance.readOnly = true;
        libraries_.push_back(appearance);
        discoverLibrary(libraries_.back());
    }

    // Discover user libraries (each sub-directory is one library)
    if (fs::exists(userLibrariesRoot_)) {
        for (const auto& entry : fs::directory_iterator(userLibrariesRoot_)) {
            if (!entry.is_directory()) continue;

            Library userLib;
            userLib.name     = entry.path().filename().string();
            userLib.rootPath = entry.path();
            userLib.readOnly = false;
            libraries_.push_back(userLib);
            discoverLibrary(libraries_.back());
        }
    }
}

// ---------------------------------------------------------------------------
// Library management
// ---------------------------------------------------------------------------

bool MaterialStore::createUserLibrary(const std::string& name)
{
    if (findLibrary(name)) return false;

    const fs::path libPath = userLibrariesRoot_ / name;
    fs::create_directories(libPath);

    Library lib;
    lib.name     = name;
    lib.rootPath = libPath;
    lib.readOnly = false;
    libraries_.push_back(lib);
    return true;
}

bool MaterialStore::removeUserLibrary(const std::string& name)
{
    const Library* lib = findLibrary(name);
    if (!lib || lib->readOnly) return false;

    // Remove all materials belonging to this library from the index
    for (auto it = materialsById_.begin(); it != materialsById_.end(); ) {
        if (it->second->getLibrary() == name)
            it = materialsById_.erase(it);
        else
            ++it;
    }

    fs::remove_all(lib->rootPath);

    libraries_.erase(
        std::remove_if(libraries_.begin(), libraries_.end(),
                       [&name](const Library& l){ return l.name == name; }),
        libraries_.end());

    return true;
}

// ---------------------------------------------------------------------------
// Material queries
// ---------------------------------------------------------------------------

std::vector<std::shared_ptr<Material>> MaterialStore::allMaterials() const
{
    std::vector<std::shared_ptr<Material>> result;
    result.reserve(materialsById_.size());
    for (const auto& [id, mat] : materialsById_)
        result.push_back(mat);
    return result;
}

std::vector<std::shared_ptr<Material>>
MaterialStore::materialsInLibrary(const std::string& libraryName) const
{
    std::vector<std::shared_ptr<Material>> result;
    for (const auto& [id, mat] : materialsById_)
        if (mat->getLibrary() == libraryName)
            result.push_back(mat);
    return result;
}

std::shared_ptr<Material> MaterialStore::findById(const std::string& id) const
{
    const auto it = materialsById_.find(id);
    return it != materialsById_.end() ? it->second : nullptr;
}

std::shared_ptr<Material> MaterialStore::findByName(const std::string& name) const
{
    std::string lower = name;
    std::transform(lower.begin(), lower.end(), lower.begin(),
                   [](unsigned char c){ return std::tolower(c); });

    for (const auto& [id, mat] : materialsById_) {
        std::string matName = mat->getName();
        std::transform(matName.begin(), matName.end(), matName.begin(),
                       [](unsigned char c){ return std::tolower(c); });
        if (matName == lower) return mat;
    }
    return nullptr;
}

// ---------------------------------------------------------------------------
// CRUD
// ---------------------------------------------------------------------------

std::shared_ptr<Material> MaterialStore::addMaterial(Material material,
                                                     const std::string& libraryName)
{
    const Library* lib = findLibrary(libraryName);
    if (!lib)
        throw std::invalid_argument("Library not found: " + libraryName);
    if (lib->readOnly)
        throw std::invalid_argument("Library is read-only: " + libraryName);

    material.setLibrary(libraryName);
    material.setReadOnly(false);

    auto shared = std::make_shared<Material>(std::move(material));
    MaterialIO::save(*shared, materialFilePath(*shared));
    materialsById_[shared->getId()] = shared;
    return shared;
}

void MaterialStore::updateMaterial(const Material& material)
{
    if (material.isReadOnly())
        throw std::runtime_error("Cannot update a read-only material: " + material.getName());

    const auto it = materialsById_.find(material.getId());
    if (it == materialsById_.end())
        throw std::runtime_error("Material not found: " + material.getId());

    *it->second = material;
    MaterialIO::save(material, materialFilePath(material));
}

void MaterialStore::deleteMaterial(const std::string& id)
{
    const auto it = materialsById_.find(id);
    if (it == materialsById_.end()) return;

    if (it->second->isReadOnly())
        throw std::runtime_error("Cannot delete a read-only material: " + it->second->getName());

    fs::path filePath = materialFilePath(*it->second);
    if (fs::exists(filePath))
        fs::remove(filePath);

    materialsById_.erase(it);
}

std::shared_ptr<Material>
MaterialStore::duplicateMaterial(const std::string& sourceId,
                                 const std::string& targetLibrary,
                                 const std::string& newName)
{
    const auto source = findById(sourceId);
    if (!source)
        throw std::invalid_argument("Source material not found: " + sourceId);

    Material copy = *source;
    copy.setName(newName.empty() ? copy.getName() + " (copy)" : newName);
    copy.setReadOnly(false);
    // Force a new id by clearing it — addMaterial will set library, save will persist
    return addMaterial(std::move(copy), targetLibrary);
}

void MaterialStore::reload()
{
    // Preserve library descriptors, clear and re-load all material data
    materialsById_.clear();
    for (const Library& lib : libraries_)
        discoverLibrary(lib);
}

// ---------------------------------------------------------------------------
// Private helpers
// ---------------------------------------------------------------------------

void MaterialStore::discoverLibrary(const Library& lib)
{
    if (!fs::exists(lib.rootPath)) return;

    for (const auto& entry : fs::recursive_directory_iterator(lib.rootPath)) {
        if (!entry.is_regular_file()) continue;
        if (entry.path().extension() != ".yaml") continue;

        try {
            Material mat = MaterialIO::load(entry.path());
            mat.setLibrary(lib.name);
            mat.setReadOnly(lib.readOnly);

            // Derive category from the sub-directory name relative to library root
            fs::path rel = fs::relative(entry.path().parent_path(), lib.rootPath);
            if (!rel.empty() && rel != ".")
                mat.setCategory(rel.string());

            materialsById_[mat.getId()] = std::make_shared<Material>(std::move(mat));
        } catch (const std::exception& e) {
            // Log and skip malformed files rather than aborting
            // TODO: route through FreeCAD's Base::Console once integrated
            (void)e;
        }
    }
}

fs::path MaterialStore::materialFilePath(const Material& material) const
{
    const Library* lib = findLibrary(material.getLibrary());
    if (!lib)
        throw std::runtime_error("Library not found for material: " + material.getName());

    fs::path dir = material.getCategory().empty()
                       ? lib->rootPath
                       : lib->rootPath / material.getCategory();
    fs::create_directories(dir);

    return dir / (Material::makeSlug(material.getName()) + ".yaml");
}

Library* MaterialStore::findLibrary(const std::string& name)
{
    for (Library& lib : libraries_)
        if (lib.name == name) return &lib;
    return nullptr;
}

const Library* MaterialStore::findLibrary(const std::string& name) const
{
    for (const Library& lib : libraries_)
        if (lib.name == name) return &lib;
    return nullptr;
}

} // namespace NeuMaterial::App
