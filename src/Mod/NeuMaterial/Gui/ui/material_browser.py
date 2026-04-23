"""
NeuMaterial library browser — a dockable panel that displays all libraries
and materials in a tree, with search filtering.
"""

from PySide6.QtCore    import Qt, Signal
from PySide6.QtWidgets import (
    QDockWidget, QWidget, QVBoxLayout, QHBoxLayout,
    QLineEdit, QTreeWidget, QTreeWidgetItem, QPushButton, QLabel,
)

from NeuMaterialApp import store as get_store


class MaterialBrowserPanel(QDockWidget):
    """Dockable panel listing all material libraries and their materials."""

    materialSelected = Signal(object)   # emits Material or None

    def __init__(self, parent=None):
        super().__init__("Material Library", parent)
        self.setObjectName("NeuMaterialBrowser")
        self.setAllowedAreas(Qt.LeftDockWidgetArea | Qt.RightDockWidgetArea)

        self._build_ui()
        self.reload()

    # ------------------------------------------------------------------
    # Public API
    # ------------------------------------------------------------------

    def selectedMaterial(self):
        """Return the currently selected Material object, or None."""
        items = self._tree.selectedItems()
        if not items:
            return None
        return items[0].data(0, Qt.UserRole)

    def reload(self):
        """Reload all libraries and materials from the store."""
        self._tree.clear()
        st = get_store()

        for lib in st.libraries():
            lib_item = QTreeWidgetItem(self._tree, [lib.name])
            lib_item.setData(0, Qt.UserRole, None)
            font = lib_item.font(0)
            font.setBold(True)
            lib_item.setFont(0, font)
            if lib.readOnly:
                lib_item.setForeground(0, lib_item.foreground(0))  # keep default
                lib_item.setToolTip(0, "Built-in library (read-only)")

            materials = st.materialsInLibrary(lib.name)
            # Group by category
            categories: dict[str, list] = {}
            for mat in materials:
                cat = mat.getCategory() or "Uncategorised"
                categories.setdefault(cat, []).append(mat)

            for cat_name, mats in sorted(categories.items()):
                cat_item = QTreeWidgetItem(lib_item, [cat_name.capitalize()])
                cat_item.setData(0, Qt.UserRole, None)
                for mat in sorted(mats, key=lambda m: m.getName()):
                    mat_item = QTreeWidgetItem(cat_item, [mat.getName()])
                    mat_item.setData(0, Qt.UserRole, mat)
                    if mat.isReadOnly():
                        mat_item.setToolTip(0, "Read-only (built-in)")
                cat_item.setExpanded(True)

            lib_item.setExpanded(True)

        self._apply_filter(self._search.text())

    # ------------------------------------------------------------------
    # Private — UI construction
    # ------------------------------------------------------------------

    def _build_ui(self):
        container = QWidget()
        layout    = QVBoxLayout(container)
        layout.setContentsMargins(4, 4, 4, 4)
        layout.setSpacing(4)

        # Search bar
        search_row = QHBoxLayout()
        search_row.addWidget(QLabel("Search:"))
        self._search = QLineEdit()
        self._search.setPlaceholderText("Filter materials…")
        self._search.setClearButtonEnabled(True)
        self._search.textChanged.connect(self._apply_filter)
        search_row.addWidget(self._search)
        layout.addLayout(search_row)

        # Tree
        self._tree = QTreeWidget()
        self._tree.setHeaderHidden(True)
        self._tree.setSelectionMode(QTreeWidget.SingleSelection)
        self._tree.itemSelectionChanged.connect(self._on_selection_changed)
        self._tree.itemDoubleClicked.connect(self._on_double_click)
        layout.addWidget(self._tree)

        # Reload button
        btn_row = QHBoxLayout()
        btn_row.addStretch()
        reload_btn = QPushButton("Reload")
        reload_btn.setFixedWidth(70)
        reload_btn.clicked.connect(self.reload)
        btn_row.addWidget(reload_btn)
        layout.addLayout(btn_row)

        self.setWidget(container)

    # ------------------------------------------------------------------
    # Private — slots
    # ------------------------------------------------------------------

    def _apply_filter(self, text: str):
        text = text.strip().lower()
        root = self._tree.invisibleRootItem()

        def _set_visible(item, visible):
            item.setHidden(not visible)

        for lib_idx in range(root.childCount()):
            lib_item   = root.child(lib_idx)
            lib_visible = False

            for cat_idx in range(lib_item.childCount()):
                cat_item    = lib_item.child(cat_idx)
                cat_visible = False

                for mat_idx in range(cat_item.childCount()):
                    mat_item = cat_item.child(mat_idx)
                    mat      = mat_item.data(0, Qt.UserRole)
                    matches  = (not text) or (text in mat.getName().lower())
                    _set_visible(mat_item, matches)
                    cat_visible = cat_visible or matches

                _set_visible(cat_item, cat_visible)
                lib_visible = lib_visible or cat_visible

            _set_visible(lib_item, lib_visible)

    def _on_selection_changed(self):
        self.materialSelected.emit(self.selectedMaterial())

    def _on_double_click(self, item, _column):
        mat = item.data(0, Qt.UserRole)
        if mat is not None:
            # Trigger edit command directly
            import FreeCADGui
            FreeCADGui.runCommand("NeuMaterial_EditMaterial")
