#include "PreCompiled.h"
#include "NeuMaterialApp.h"
#include "MaterialStore.h"

#include <Base/Console.h>
#include <Base/Interpreter.h>
#include <App/Application.h>

namespace NeuMaterial::App {

void initializeApp()
{
    Base::Console().Log("NeuMaterialApp: initializing\n");

    // Resolve the workbench module directory so we can locate Resources/.
    // App::Application::getResourceDir() returns the FreeCAD resource root;
    // our module lives under Mod/NeuMaterial/ relative to that.
    namespace fs = std::filesystem;

    fs::path modDir = fs::path(App::Application::getResourceDir())
                      / "Mod" / "NeuMaterial";
    fs::path resourcesPath = modDir / "Resources";

    // Initialize the global material store.
    // User libraries default to the XDG/platform-appropriate location.
    MaterialStore::instance().initialise(resourcesPath);

    Base::Console().Log("NeuMaterialApp: material store ready (resources: %s)\n",
                        resourcesPath.string().c_str());
}

} // namespace NeuMaterial::App

// ---------------------------------------------------------------------------
// Singleton accessor — makes the store available to Python bindings and Gui
// ---------------------------------------------------------------------------

MaterialStore& MaterialStore::instance()
{
    static MaterialStore store;
    return store;
}

} // namespace NeuMaterial::App

/// ---------------------------------------------------------------------------
// CPython module entry point
// FreeCAD discovers this via the shared library's exported symbol.
// ---------------------------------------------------------------------------

extern "C" {

void initNeuMaterialApp()
{
    // Protect against multiple initialization (e.g., reload in Python)
    static bool initialized = false;
    if (initialized) return;
    initialized = true;

    NeuMaterial::App::initializeApp();
}

} // extern "C"
