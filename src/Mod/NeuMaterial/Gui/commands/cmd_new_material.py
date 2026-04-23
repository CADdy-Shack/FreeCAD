"""Command: create a new material via the editor dialog."""

import FreeCAD
import FreeCADGui

from ..ui.material_editor import MaterialEditorDialog
from NeuMaterialApp import Material, store   # C++ bindings


class NewMaterialCommand:
    """Create a new material and add it to a user library."""

    def GetResources(self):
        return {
            "Pixmap":   "new_material",
            "MenuText": "New Material…",
            "ToolTip":  "Create a new material and add it to a user library.",
            "Accel":    "Ctrl+Shift+N",
        }

    def IsActive(self):
        return True

    def Activated(self):
        dlg = MaterialEditorDialog(
            parent=FreeCADGui.getMainWindow(),
            material=None,       # None → create mode
        )
        if dlg.exec() != dlg.Accepted:
            return

        material, library_name = dlg.result()
        try:
            store().addMaterial(material, library_name)
            FreeCAD.Console.PrintMessage(
                f"NeuMaterial: created '{material.getName()}' in '{library_name}'\n"
            )
        except Exception as exc:
            from PySide6.QtWidgets import QMessageBox
            QMessageBox.critical(
                FreeCADGui.getMainWindow(),
                "NeuMaterial",
                f"Could not create material:\n{exc}",
            )
