# SPDX-FileCopyrightText: 2026 Michael Cummings <mgcummings@yahoo.com>
# SPDX-License-Identifier: CC-BY-4.0

"""Initialization of the NeuMaterial Workbench graphical interface."""

import os

import FreeCAD as App
import FreeCADGui as Gui


class NeuMaterialWorkbench(Gui.Workbench):
    """NeuMaterial workbench object."""

    def __init__(self):
        self.__class__.Icon = os.path.join(
            App.getResourceDir(),
            "Mod", "NeuMaterial",
            "Gui", "Resources", "icons",
            "NeuMaterial.svg",
        )
        self.__class__.MenuText = "NeuMaterial"
        self.__class__.ToolTip  = "Material physical properties and appearance management"

    def Initialize(self):
        import NeuMaterialGui as _  # noqa: F401 — imported for side effects only

    def GetClassName(self):
        return "NeuMaterial::Gui::NeuMaterialWorkbench"


Gui.addWorkbench(NeuMaterialWorkbench())

App.__unit_test__ += ["TestNeuMaterialGui"]
