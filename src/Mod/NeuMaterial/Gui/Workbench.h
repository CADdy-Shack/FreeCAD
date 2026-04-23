#pragma once

#include <Gui/Workbench.h>

namespace NeuMaterial::Gui {

// ---------------------------------------------------------------------------
// NeuMaterialWorkbench
//
// Registers the workbench with FreeCAD's GUI system.
// Defines the toolbar, menu, and context menu layout.
// ---------------------------------------------------------------------------

class NeuMaterialWorkbench : public Gui::StdWorkbench {
    TYPESYSTEM_HEADER_WITH_OVERRIDE();

public:
    NeuMaterialWorkbench()  = default;
    ~NeuMaterialWorkbench() = default;

protected:
    Gui::MenuItem*  setupMenuBar()    override;
    Gui::ToolBarItem* setupToolBars() override;
    Gui::ToolBarItem* setupCommandBars() override;
};

} // namespace NeuMaterial::Gui
