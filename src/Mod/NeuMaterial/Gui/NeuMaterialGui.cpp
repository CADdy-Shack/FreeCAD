#include "PreCompiled.h"
#include "NeuMaterialGui.h"
#include "Workbench.h"

#include <Base/Console.h>
#include <Gui/Application.h>

namespace NeuMaterial::Gui {

void initializeGui()
{
    // Register the workbench type so FreeCAD can instantiate it by name
    NeuMaterialWorkbench::init();

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
