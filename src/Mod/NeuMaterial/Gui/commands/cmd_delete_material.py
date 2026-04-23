"""Command: delete the selected user-library material."""

import FreeCAD
import FreeCADGui
from PySide6.QtWidgets import QMessageBox

from ..ui.material_browser import MaterialBrowserPanel
from NeuMaterialApp import store


class DeleteMaterialCommand:
    """Delete the selected material (user libraries only)."""

    def GetResources(self):
        return {
            "Pixmap":   "delete_material",
            "MenuText": "Delete Material",
            "ToolTip":  "Permanently delete the selected material from its user library.",
        }

    def IsActive(self):
        mat = self._selected_material()
        # Only active when a non-read-only material is selected
        return mat is not None and not mat.isReadOnly()

    def Activated(self):
        mw  = FreeCADGui.getMainWindow()
        mat = self._selected_material()
        if mat is None:
            return

        confirm = QMessageBox.question(
            mw,
            "Delete Material",
            f"Permanently delete '{mat.getName()}'?\n\nThis cannot be undone.",
            QMessageBox.Yes | QMessageBox.No,
            QMessageBox.No,
        )
        if confirm != QMessageBox.Yes:
            return

        try:
            store().deleteMaterial(mat.getId())
            FreeCAD.Console.PrintMessage(
                f"NeuMaterial: deleted '{mat.getName()}'\n"
            )
            browser = mw.findChild(MaterialBrowserPanel)
            if browser:
                browser.reload()
        except Exception as exc:
            QMessageBox.critical(mw, "NeuMaterial",
                                 f"Could not delete material:\n{exc}")

    @staticmethod
    def _selected_material():
        mw = FreeCADGui.getMainWindow()
        browser = mw.findChild(MaterialBrowserPanel)
        return browser.selectedMaterial() if browser else None
