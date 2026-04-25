#include "MaterialStore.h"
#include "MaterialIO.h"

#include <random>
#include <iomanip>

#include <Base/Console.h>

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
    wchar_t path[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathW(nullptr, CSIDL_APPDATA, nullptr, 0, path)))
        return fs::path(path) / "FreeCAD" / "NeuMaterial" / "libraries";
    return fs::temp_directory_path() / "NeuMaterial" / "libraries";
#elif defined(__APPLE__)
    const char* home = getenv("HOME");
    if (!home) {
        struct passwd* pw = getpwuid(getuid());
        home = pw ? pw->pw_dir : "/tmp";
    }
    return fs::path(home) / "Library" / "Application Support"
                          / "FreeCAD" / "NeuMaterial" / "libraries";
#else
    // XDG: $XDG_DATA_HOME/FreeCAD/NeuMaterial/libraries
    const char* xdg = getenv("XDG_DATA_HOME");
    if (xdg && *xdg)
        return fs::path(xdg) / "FreeCAD" / "NeuMaterial" / "libraries";
    const char* home = getenv("HOME");
    if (!home) {
        struct passwd* pw = getpwuid(getuid());
        home = pw ? pw->pw_dir : "/tmp";
    }
    return fs::path(home) / ".local" / "share"
                          / "FreeCAD" / "NeuMaterial" / "libraries";
#endif
}

// ---------------------------------------------------------------------------
// Initialization
// ---------------------------------------------------------------------------

void MaterialStore::initialize(const fs::path& resourcesPath,
                               const fs::path& userLibrariesRoot)
{
    libraries_.clear();
    byUuid_.clear();

    userLibrariesRoot_ = userLibrariesRoot.empty()
                             ? defaultUserLibrariesRoot()
                             : userLibrariesRoot;

    // Built-in Standard library — Resources/Material/Standard/
    {
        Library lib;
        lib.name     = "Standard";
        lib.readOnly = true;
        lib.rootPath = resourcesPath / "Material" / "Standard";
        libraries_.push_back(lib);
        discoverLibrary(libraries_.back());
    }

    // Built-in Appearance library — Resources/Material/Appearance/
    {
        Library lib;
        lib.name     = "Appearance";
        lib.readOnly = true;
        lib.rootPath = resourcesPath / "Material" / "Appearance";
        libraries_.push_back(lib);
        discoverLibrary(libraries_.back());
    }

    // User libraries — one sub-directory per library
    if (fs::exists(userLibrariesRoot_)) {
        for (const auto& entry :
             fs::directory_iterator(userLibrariesRoot_)) {
            if (!entry.is_directory()) continue;
            Library lib;
            lib.name     = entry.path().filename().string();
            lib.readOnly = false;
            lib.rootPath = entry.path();
            libraries_.push_back(lib);
            discoverLibrary(libraries_.back());
        }
    }

    Base::Console().Log(
        "NeuMaterial: MaterialStore ready — %zu material(s) in %zu library(-ies)\n",
        byUuid_.size(), libraries_.size());
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
    lib.readOnly = false;
    lib.rootPath = libPath;
    libraries_.push_back(lib);
    return true;
}

bool MaterialStore::removeUserLibrary(const std::string& name)
{
    Library* lib = findLibrary(name);
    if (!lib || lib->readOnly) return false;

    for (auto it = byUuid_.begin(); it != byUuid_.end(); ) {
        if (it->second->getLibrary() == name)
            it = byUuid_.erase(it);
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
    result.reserve(byUuid_.size());
    for (const auto& [uuid, mat] : byUuid_)
        result.push_back(mat);
    return result;
}

std::vector<std::shared_ptr<Material>>
MaterialStore::materialsInLibrary(const std::string& libraryName) const
{
    std::vector<std::shared_ptr<Material>> result;
    for (const auto& [uuid, mat] : byUuid_)
        if (mat->getLibrary() == libraryName)
            result.push_back(mat);
    return result;
}

std::shared_ptr<Material>
MaterialStore::findByUuid(const std::string& uuid) const
{
    const auto it = byUuid_.find(uuid);
    return it != byUuid_.end() ? it->second : nullptr;
}

std::shared_ptr<Material>
MaterialStore::findByName(const std::string& name) const
{
    std::string lower = name;
    std::transform(lower.begin(), lower.end(), lower.begin(),
                   [](unsigned char c){ return std::tolower(c); });

    for (const auto& [uuid, mat] : byUuid_) {
        std::string n = mat->getName();
        std::transform(n.begin(), n.end(), n.begin(),
                       [](unsigned char c){ return std::tolower(c); });
        if (n == lower) return mat;
    }
    return nullptr;
}

// ---------------------------------------------------------------------------
// CRUD
// ---------------------------------------------------------------------------

std::shared_ptr<Material>
MaterialStore::addMaterial(Material material, const std::string& libraryName)
{
    if (material.getUuid().empty())
        throw std::invalid_argument(
            "Material must have a UUID before calling addMaterial().");
    if (material.getName().empty())
        throw std::invalid_argument(
            "Material must have a Name before calling addMaterial().");

    const Library* lib = findLibrary(libraryName);
    if (!lib)
        throw std::invalid_argument("Library not found: " + libraryName);
    if (lib->readOnly)
        throw std::invalid_argument("Library is read-only: " + libraryName);

    material.setLibrary(libraryName);
    material.setReadOnly(false);

    auto shared = std::make_shared<Material>(std::move(material));
    MaterialIO::save(*shared, materialFilePath(*shared));
    byUuid_[shared->getUuid()] = shared;
    return shared;
}

void MaterialStore::updateMaterial(const Material& material)
{
    if (material.isReadOnly())
        throw std::runtime_error(
            "Cannot update read-only material: " + material.getName());

    const auto it = byUuid_.find(material.getUuid());
    if (it == byUuid_.end())
        throw std::runtime_error(
            "Material not found in store: " + material.getUuid());

    *it->second = material;
    MaterialIO::save(material, materialFilePath(material));
}

void MaterialStore::deleteMaterial(const std::string& uuid)
{
    const auto it = byUuid_.find(uuid);
    if (it == byUuid_.end()) return;

    if (it->second->isReadOnly())
        throw std::runtime_error(
            "Cannot delete read-only material: " + it->second->getName());

    const fs::path path = materialFilePath(*it->second);
    if (fs::exists(path))
        fs::remove(path);

    byUuid_.erase(it);
}

std::shared_ptr<Material>
MaterialStore::duplicateMaterial(const std::string& sourceUuid,
                                 const std::string& targetLibrary,
                                 const std::string& newName)
{
    const auto source = findByUuid(sourceUuid);
    if (!source)
        throw std::invalid_argument(
            "Source material not found: " + sourceUuid);

    Material copy = *source;
    copy.setName(newName.empty() ? copy.getName() + " (copy)" : newName);
    copy.setUuid(generateUuid());
    copy.setReadOnly(false);
    return addMaterial(std::move(copy), targetLibrary);
}

void MaterialStore::reload()
{
    byUuid_.clear();
    for (const Library& lib : libraries_)
        discoverLibrary(lib);
}

// ---------------------------------------------------------------------------
// Private helpers
// ---------------------------------------------------------------------------

void MaterialStore::discoverLibrary(const Library& lib)
{
    if (!fs::exists(lib.rootPath)) return;

    for (const auto& entry :
         fs::recursive_directory_iterator(lib.rootPath)) {
        if (!entry.is_regular_file()) continue;
        if (entry.path().extension() != ".yaml") continue;

        try {
            Material mat = MaterialIO::load(entry.path());

            if (mat.getUuid().empty()) {
                Base::Console().Warning(
                    "NeuMaterial: skipping material with no UUID: %s\n",
                    entry.path().string().c_str());
                continue;
            }

            if (byUuid_.count(mat.getUuid())) {
                Base::Console().Warning(
                    "NeuMaterial: duplicate UUID '%s' in %s — skipping\n",
                    mat.getUuid().c_str(),
                    entry.path().string().c_str());
                continue;
            }

            mat.setLibrary(lib.name);
            mat.setReadOnly(lib.readOnly);

            // Derive category from sub-directory path relative to library root
            const fs::path rel = fs::relative(
                entry.path().parent_path(), lib.rootPath);
            if (!rel.empty() && rel != ".")
                mat.setCategory(rel.string());

            byUuid_[mat.getUuid()] = std::make_shared<Material>(std::move(mat));
        }
        catch (const std::exception& e) {
            Base::Console().Warning(
                "NeuMaterial: failed to load material %s: %s\n",
                entry.path().string().c_str(), e.what());
        }
    }
}

fs::path MaterialStore::materialFilePath(const Material& material) const
{
    const Library* lib = findLibrary(material.getLibrary());
    if (!lib)
        throw std::runtime_error(
            "Library not found for material: " + material.getName());

    // User material files are named by UUID to avoid slug collisions
    const fs::path dir = material.getCategory().empty()
                             ? lib->rootPath
                             : lib->rootPath / material.getCategory();
    fs::create_directories(dir);

    return dir / (material.getUuid() + ".yaml");
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

std::string MaterialStore::generateUuid()
{
    std::random_device              rd;
    std::mt19937                    gen(rd());
    std::uniform_int_distribution<> d(0, 15);
    std::uniform_int_distribution<> d2(8, 11);

    std::ostringstream oss;
    oss << std::hex;
    for (int i = 0; i < 8;  ++i) oss << d(gen);  oss << '-';
    for (int i = 0; i < 4;  ++i) oss << d(gen);  oss << "-4";
    for (int i = 0; i < 3;  ++i) oss << d(gen);  oss << '-';
    oss << d2(gen);
    for (int i = 0; i < 3;  ++i) oss << d(gen);  oss << '-';
    for (int i = 0; i < 12; ++i) oss << d(gen);
    return oss.str();
}

} // namespace NeuMaterial::App
