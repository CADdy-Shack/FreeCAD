"""Command: open (or focus) the NeuMaterial library browser panel."""

import FreeCADGui
from ..ui.material_browser import MaterialBrowserPanel


class BrowseLibraryCommand:
    """Show the NeuMaterial library browser."""

    def GetResources(self):
        return {
            "Pixmap":   "browse_library",
            "MenuText": "Browse Material Library",
            "ToolTip":  "Open the NeuMaterial library browser panel.",
            "Accel":    "Shift+M",
        }

    def IsActive(self):
        return True  # always available regardless of document state

    def Activated(self):
        mw = FreeCADGui.getMainWindow()
        # Reuse an existing panel if one is already docked
        existing = mw.findChild(MaterialBrowserPanel)
        if existing:
            existing.setVisible(True)
            existing.raise_()
            return

        panel = MaterialBrowserPanel(mw)
        mw.addDockWidget(0x2, panel)   # Qt.RightDockWidgetArea = 0x2
        panel.show()
