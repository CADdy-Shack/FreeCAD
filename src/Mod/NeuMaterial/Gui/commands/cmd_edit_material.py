"""Command: edit the currently selected material."""

import FreeCAD
import FreeCADGui
from PySide6.QtWidgets import QMessageBox

from ..ui.material_editor import MaterialEditorDialog
from ..ui.material_browser import MaterialBrowserPanel
from NeuMaterialApp import store


class EditMaterialCommand:
    """Edit the selected material's properties."""

    def GetResources(self):
        return {
            "Pixmap":   "edit_material",
            "MenuText": "Edit Material…",
            "ToolTip":  "Edit the selected material's properties.",
        }

    def IsActive(self):
        return self._selected_material() is not None

    def Activated(self):
        material = self._selected_material()
        if material is None:
            return

        if material.isReadOnly():
            QMessageBox.information(
                FreeCADGui.getMainWindow(),
                "NeuMaterial",
                "Built-in materials are read-only.\n"
                "Use 'Duplicate Material' to create an editable copy.",
            )
            return

        dlg = MaterialEditorDialog(
            parent=FreeCADGui.getMainWindow(),
            material=material,
        )
        if dlg.exec() != dlg.Accepted:
            return

        updated, _library = dlg.result()
        try:
            store().updateMaterial(updated)
            FreeCAD.Console.PrintMessage(
                f"NeuMaterial: updated '{updated.getName()}'\n"
            )
        except Exception as exc:
            QMessageBox.critical(
                FreeCADGui.getMainWindow(),
                "NeuMaterial",
                f"Could not update material:\n{exc}",
            )

    # ------------------------------------------------------------------

    @staticmethod
    def _selected_material():
        """Return the material currently selected in the browser, or None."""
        mw = FreeCADGui.getMainWindow()
        browser = mw.findChild(MaterialBrowserPanel)
        if browser:
            return browser.selectedMaterial()
        return None
