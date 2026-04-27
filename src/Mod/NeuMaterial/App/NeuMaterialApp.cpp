// SPDX-FileCopyrightText: 2026 Michael Cummings <mgcummings@yahoo.com>
// SPDX-License-Identifier: CC-BY-4.0

#include "PreCompiled.h"
#include "NeuMaterialApp.h"
#include "ModelStore.h"
#include "MaterialStore.h"
#include "MaterialStorePy.h"
#include "ModelStorePy.h"

#include <Base/Console.h>
#include <App/Application.h>

namespace NeuMaterial::App {

// ---------------------------------------------------------------------------
// Singletons
// ---------------------------------------------------------------------------

ModelStore& modelStore()
{
    static ModelStore store;
    return store;
}

MaterialStore& materialStore()
{
    static MaterialStore store;
    return store;
}

// ---------------------------------------------------------------------------
// Initialization
// ---------------------------------------------------------------------------

void initializeApp()
{
    namespace fs = std::filesystem;

    const fs::path modDir = fs::path(App::Application::getResourceDir())
                            / "Mod" / "NeuMaterial";
    const fs::path resourcesPath = modDir / "Resources";

    // ModelStore must be initialized first — MaterialValidator and
    // MaterialIO depend on it being ready before any material is loaded.
    modelStore().initialize(resourcesPath / "Models");

    // MaterialStore second — loads material YAMLs which reference model UUIDs
    materialStore().initialize(resourcesPath);

    Base::Console().Log("NeuMaterialApp: ready (resources: %s)\n",
                        resourcesPath.string().c_str());
}

} // namespace NeuMaterial::App

// ---------------------------------------------------------------------------
// CPython module-level functions
// Exposed as NeuMaterialApp.materialStore() and NeuMaterialApp.modelStore()
// ---------------------------------------------------------------------------

static PyObject* py_materialStore(PyObject* /*self*/, PyObject* /*args*/)
{
    return new MaterialStorePy(&NeuMaterial::App::materialStore());
}

static PyObject* py_modelStore(PyObject* /*self*/, PyObject* /*args*/)
{
    return new ModelStorePy(&NeuMaterial::App::modelStore());
}

static PyMethodDef NeuMaterialAppMethods[] = {
    {"materialStore", py_materialStore, METH_NOARGS,
     "Return the process-wide MaterialStore singleton."},
    {"modelStore",    py_modelStore,    METH_NOARGS,
     "Return the process-wide ModelStore singleton."},
    {nullptr, nullptr, 0, nullptr}
};

// ---------------------------------------------------------------------------
// CPython module entry point
// ---------------------------------------------------------------------------

extern "C" {

void initNeuMaterialApp()
{
    static bool initialized = false;
    if (initialized) return;
    initialized = true;

    NeuMaterial::App::initializeApp();

    // Register module-level functions with the already-created module
    // (FreeCAD's module loader creates the module object before calling init)
    PyObject* mod = PyImport_ImportModule("NeuMaterialApp");
    if (mod) {
        for (auto* def = NeuMaterialAppMethods; def->ml_name; ++def) {
            PyObject* fn = PyCFunction_New(def, nullptr);
            PyModule_AddObject(mod, def->ml_name, fn);
        }
        Py_DECREF(mod);
    }
}

} // extern "C"
