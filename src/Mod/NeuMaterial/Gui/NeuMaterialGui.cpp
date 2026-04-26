// SPDX-FileCopyrightText: 2026 Michael Cummings <mgcummings@yahoo.com>
// SPDX-License-Identifier: CC-BY-4.0

#include "NeuMaterialGui.h"
#include "Workbench.h"

#include <Base/Console.h>
#include <Base/Interpreter.h>
#include <Gui/Application.h>

namespace NeuMaterial::Gui {

void initializeGui()
{
    // Register the C++ workbench type so FreeCAD can instantiate it
    // via GetClassName() = "NeuMaterial::Gui::NeuMaterialWorkbench"
    NeuMaterialWorkbench::init();

    // Import the Python commands package — registers all FreeCAD commands
    // via FreeCADGui.addCommand() so they are available in toolbars and menus
    Base::Interpreter().runString(
        "import NeuMaterial.Gui.commands"
    );

    Base::Console().Log("NeuMaterialGui: initialized\n");
}

} // namespace NeuMaterial::Gui

extern "C" {

void initNeuMaterialGui()
{
    static bool initialized = false;
    if (initialized) return;
    initialized = true;

    NeuMaterial::Gui::initializeGui();
}

} // extern "C"
