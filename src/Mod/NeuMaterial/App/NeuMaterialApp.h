#pragma once

// ---------------------------------------------------------------------------
// NeuMaterialApp
//
// Entry point for the App (non-GUI) shared library.
// Exposes the Python module initialisation function so FreeCAD can import
// NeuMaterialApp as a Python extension.
// ---------------------------------------------------------------------------

// Forward-declare the CPython init function required by every extension module.
// The actual definition is in NeuMaterialApp.cpp.
extern "C" void initNeuMaterialApp();

namespace NeuMaterial::App {

/// Called once when the App module is first imported.
/// Locates Resources/, initializes the MaterialStore, and registers any
/// document types needed by FreeCAD.
void initializeApp();

} // namespace NeuMaterial::App
