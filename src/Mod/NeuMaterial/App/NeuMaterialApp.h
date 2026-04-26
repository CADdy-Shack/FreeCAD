#pragma once

// ---------------------------------------------------------------------------
// NeuMaterialApp
//
// Entry point for the App (non-GUI) shared library.
// Exposes the Python module initialization function so FreeCAD can import
// NeuMaterialApp as a Python extension, and declares the process-wide
// singleton accessors for ModelStore and MaterialStore.
// ---------------------------------------------------------------------------

// Forward declarations — avoid pulling in full headers here
namespace NeuMaterial::App {
class MaterialStore;
class ModelStore;
}

extern "C" void initNeuMaterialApp();

namespace NeuMaterial::App {

/// Called once when the App module is first imported.
/// Initializes ModelStore then MaterialStore from Resources/.
void initializeApp();

/// Process-wide MaterialStore singleton.
/// Available after initializeApp() returns.
MaterialStore& materialStore();

/// Process-wide ModelStore singleton.
/// Available after initializeApp() returns.
ModelStore& modelStore();

} // namespace NeuMaterial::App
