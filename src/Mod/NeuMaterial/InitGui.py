# InitGui.py — loaded by FreeCAD only when the GUI is active.
# Registers the NeuMaterial workbench.

import FreeCADGui

FreeCADGui.addWorkbench("NeuMaterialWorkbench")

FreeCAD.Console.PrintLog("NeuMaterial: Gui module loaded\n")
