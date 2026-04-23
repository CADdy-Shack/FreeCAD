"""Command: duplicate a material (including read-only) into a user library."""

import FreeCAD
import FreeCADGui
from PySide6.QtWidgets import QInputDialog, QMessageBox

from ..ui.material_browser import MaterialBrowserPanel
from NeuMaterialApp import store


class DuplicateMaterialCommand:
    """Duplicate the selected material into a user library."""

    def GetResources(self):
        return {
            "Pixmap":   "duplicate_material",
            "MenuText": "Duplicate Material…",
            "ToolTip":  "Copy the selected material into a user library for editing.",
        }

    def IsActive(self):
        mw = FreeCADGui.getMainWindow()
        browser = mw.findChild(MaterialBrowserPanel)
        return browser is not None and browser.selectedMaterial() is not None

    def Activated(self):
        mw = FreeCADGui.getMainWindow()
        browser = mw.findChild(MaterialBrowserPanel)
        source = browser.selectedMaterial() if browser else None
        if source is None:
            return

        st = store()

        # Collect user-writable libraries as target choices
        user_libs = [lib.name for lib in st.libraries() if not lib.readOnly]
        if not user_libs:
            QMessageBox.information(
                mw, "NeuMaterial",
                "No user libraries found.\n"
                "Create a user library first via File > New Library.",
            )
            return

        target_lib, ok = QInputDialog.getItem(
            mw,
            "Duplicate Material",
            "Copy into library:",
            user_libs,
            0,
            False,
        )
        if not ok:
            return

        new_name, ok = QInputDialog.getText(
            mw,
            "Duplicate Material",
            "New material name:",
            text=source.getName() + " (copy)",
        )
        if not ok or not new_name.strip():
            return

        try:
            new_mat = st.duplicateMaterial(source.getId(), target_lib, new_name.strip())
            FreeCAD.Console.PrintMessage(
                f"NeuMaterial: duplicated '{source.getName()}' "
                f"→ '{new_mat.getName()}' in '{target_lib}'\n"
            )
            if browser:
                browser.reload()
        except Exception as exc:
            QMessageBox.critical(mw, "NeuMaterial",
                                 f"Could not duplicate material:\n{exc}")
