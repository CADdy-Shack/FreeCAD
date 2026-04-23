"""
MaterialEditorDialog — modal dialog for creating and editing materials.
Works in two modes:
  - Create mode  (material=None): blank form, user picks a target library
  - Edit mode    (material=<obj>): pre-populated form, library fixed
"""

from PySide6.QtCore    import Qt
from PySide6.QtWidgets import (
    QDialog, QDialogButtonBox, QVBoxLayout, QHBoxLayout,
    QFormLayout, QLineEdit, QComboBox, QScrollArea,
    QWidget, QLabel, QTabWidget, QMessageBox,
)

from NeuMaterialApp import Material, store as get_store
from .property_widgets import (
    MechanicalWidget, ThermalWidget, ElectricalWidget, AppearanceWidget,
)


class MaterialEditorDialog(QDialog):
    """Create or edit a material."""

    def __init__(self, parent=None, material=None):
        super().__init__(parent)
        self._source   = material      # None → create mode
        self._result   = None          # set on accept
        self._is_edit  = material is not None

        self.setWindowTitle("Edit Material" if self._is_edit else "New Material")
        self.setMinimumSize(480, 600)
        self.resize(520, 680)

        self._build_ui()

        if self._is_edit:
            self._populate(material)

    # ------------------------------------------------------------------
    # Public
    # ------------------------------------------------------------------

    def result(self):
        """Return (Material, library_name) after accepted exec_()."""
        return self._result

    # ------------------------------------------------------------------
    # UI construction
    # ------------------------------------------------------------------

    def _build_ui(self):
        layout = QVBoxLayout(self)
        layout.setSpacing(8)

        # --- Identity fields ---
        id_form = QFormLayout()
        id_form.setLabelAlignment(Qt.AlignRight)

        self._name_edit = QLineEdit()
        self._name_edit.setPlaceholderText("Required")
        id_form.addRow("Name *", self._name_edit)

        self._desc_edit = QLineEdit()
        self._desc_edit.setPlaceholderText("Optional description")
        id_form.addRow("Description", self._desc_edit)

        # Category
        self._cat_edit = QLineEdit()
        self._cat_edit.setPlaceholderText("e.g. metals, polymers")
        id_form.addRow("Category", self._cat_edit)

        # Library selector (only in create mode)
        st         = get_store()
        user_libs  = [lib.name for lib in st.libraries() if not lib.readOnly]

        if not self._is_edit:
            self._lib_combo = QComboBox()
            for lib in user_libs:
                self._lib_combo.addItem(lib)
            id_form.addRow("Library *", self._lib_combo)
        else:
            self._lib_label = QLabel(self._source.getLibrary())
            id_form.addRow("Library", self._lib_label)

        layout.addLayout(id_form)

        # --- Property tabs ---
        tabs = QTabWidget()

        self._mech_widget = MechanicalWidget()
        self._therm_widget = ThermalWidget()
        self._elec_widget  = ElectricalWidget()
        self._app_widget   = AppearanceWidget()

        def _scroll(w):
            area = QScrollArea()
            area.setWidgetResizable(True)
            area.setWidget(w)
            return area

        tabs.addTab(_scroll(self._mech_widget),  "Mechanical")
        tabs.addTab(_scroll(self._therm_widget), "Thermal")
        tabs.addTab(_scroll(self._elec_widget),  "Electrical")
        tabs.addTab(_scroll(self._app_widget),   "Appearance")

        layout.addWidget(tabs, stretch=1)

        # --- Buttons ---
        buttons = QDialogButtonBox(
            QDialogButtonBox.Ok | QDialogButtonBox.Cancel
        )
        buttons.accepted.connect(self._on_accept)
        buttons.rejected.connect(self.reject)
        layout.addWidget(buttons)

    # ------------------------------------------------------------------
    # Populate (edit mode)
    # ------------------------------------------------------------------

    def _populate(self, mat):
        self._name_edit.setText(mat.getName())
        self._desc_edit.setText(mat.getDescription())
        self._cat_edit.setText(mat.getCategory())

        self._mech_widget.populate(mat.mechanical())
        self._therm_widget.populate(mat.thermal())
        self._elec_widget.populate(mat.electrical())
        self._app_widget.populate(mat.appearance())

        if mat.isReadOnly():
            self._name_edit.setReadOnly(True)
            self._desc_edit.setReadOnly(True)
            self._cat_edit.setReadOnly(True)

    # ------------------------------------------------------------------
    # Accept handler
    # ------------------------------------------------------------------

    def _on_accept(self):
        name = self._name_edit.text().strip()
        if not name:
            QMessageBox.warning(self, "NeuMaterial", "Material name is required.")
            return

        # Build / update Material object
        if self._is_edit:
            mat = self._source
            mat.setName(name)
            mat.setDescription(self._desc_edit.text().strip())
            mat.setCategory(self._cat_edit.text().strip())
            library = mat.getLibrary()
        else:
            mat = Material(name)
            mat.setDescription(self._desc_edit.text().strip())
            mat.setCategory(self._cat_edit.text().strip())
            library = self._lib_combo.currentText()

        # Collect property groups
        self._mech_widget.collect(mat.mechanical())
        self._therm_widget.collect(mat.thermal())
        self._elec_widget.collect(mat.electrical())
        self._app_widget.collect(mat.appearance())

        # Validate
        from NeuMaterialApp import MaterialValidator
        result = MaterialValidator.validate(mat)
        if not result.isValid():
            QMessageBox.critical(
                self, "Validation Error",
                "Please fix the following errors:\n\n" + result.errorSummary(),
            )
            return

        if result.hasWarnings():
            warn_msgs = "\n".join(
                f"• [{i.field}] {i.message}"
                for i in result.issues
                if i.severity.name == "Warning"
            )
            reply = QMessageBox.warning(
                self, "Validation Warnings",
                f"The following warnings were found:\n\n{warn_msgs}\n\nSave anyway?",
                QMessageBox.Yes | QMessageBox.No,
                QMessageBox.No,
            )
            if reply != QMessageBox.Yes:
                return

        self._result = (mat, library)
        self.accept()
