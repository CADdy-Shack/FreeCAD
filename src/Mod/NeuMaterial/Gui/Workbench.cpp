#include "PreCompiled.h"
#include "Workbench.h"

#include <Gui/MenuManager.h>
#include <Gui/ToolBarManager.h>

namespace NeuMaterial::Gui {

TYPESYSTEM_SOURCE(NeuMaterial::Gui::NeuMaterialWorkbench, Gui::StdWorkbench);

// ---------------------------------------------------------------------------
// Menu bar
// ---------------------------------------------------------------------------

Gui::MenuItem* NeuMaterialWorkbench::setupMenuBar()
{
    Gui::MenuItem* root = StdWorkbench::setupMenuBar();

    Gui::MenuItem* materials = new Gui::MenuItem;
    root->insertItem(root->item("&Windows"), materials);
    materials->setCommand("&Materials");

    *materials
        << "NeuMaterial_BrowseLibrary"
        << "Separator"
        << "NeuMaterial_NewMaterial"
        << "NeuMaterial_EditMaterial"
        << "NeuMaterial_DuplicateMaterial"
        << "NeuMaterial_DeleteMaterial";

    return root;
}

// ---------------------------------------------------------------------------
// Toolbars
// ---------------------------------------------------------------------------

Gui::ToolBarItem* NeuMaterialWorkbench::setupToolBars()
{
    Gui::ToolBarItem* root = StdWorkbench::setupToolBars();

    Gui::ToolBarItem* bar = new Gui::ToolBarItem(root);
    bar->setCommand("NeuMaterial");

    *bar
        << "NeuMaterial_BrowseLibrary"
        << "Separator"
        << "NeuMaterial_NewMaterial"
        << "NeuMaterial_EditMaterial"
        << "NeuMaterial_DuplicateMaterial"
        << "NeuMaterial_DeleteMaterial";

    return root;
}

Gui::ToolBarItem* NeuMaterialWorkbench::setupCommandBars()
{
    return StdWorkbench::setupCommandBars();
}

} // namespace NeuMaterial::Gui
