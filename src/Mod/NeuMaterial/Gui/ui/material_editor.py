"""
MaterialEditorDialog — modal dialog for creating and editing materials.

Two modes:
  - Create (material=None): blank form, user picks target library,
    store generates UUID on save.
  - Edit (material=<obj>): pre-populated form, library fixed.
"""

import uuid as _uuid_mod

from PySide6.QtCore    import Qt
from PySide6.QtWidgets import (
    QComboBox, QDialog, QDialogButtonBox, QFormLayout,
    QHBoxLayout, QLabel, QLineEdit, QMessageBox,
    QScrollArea, QTabWidget, QVBoxLayout, QWidget,
)

from NeuMaterialApp import Material, store as get_store
from .property_widgets import (
    AppearanceWidget, ElectricalWidget, MechanicalWidget, ThermalWidget,
)


class MaterialEditorDialog(QDialog):
    """Create or edit a material."""

    def __init__(self, parent=None, material=None):
        super().__init__(parent)
        self._source  = material
        self._result  = None
        self._is_edit = material is not None

        self.setWindowTitle("Edit Material" if self._is_edit else "New Material")
        self.setMinimumSize(520, 640)
        self.resize(560, 720)

        self._build_ui()

        if self._is_edit:
            self._populate(material)

    # ------------------------------------------------------------------
    # Public
    # ------------------------------------------------------------------

    def result(self):
        """Return (Material, library_name) after accepted exec()."""
        return self._result

    # ------------------------------------------------------------------
    # UI construction
    # ------------------------------------------------------------------

    def _build_ui(self):
        layout = QVBoxLayout(self)
        layout.setSpacing(8)

        # General fields — sorted a-z to match YAML output
        gen_form = QFormLayout()
        gen_form.setLabelAlignment(Qt.AlignRight)

        self._author_edit  = QLineEdit()
        self._author_edit.setPlaceholderText("Your name")
        gen_form.addRow("Author", self._author_edit)

        self._desc_edit = QLineEdit()
        self._desc_edit.setPlaceholderText("Optional description")
        gen_form.addRow("Description", self._desc_edit)

        self._license_edit = QLineEdit()
        self._license_edit.setPlaceholderText("e.g. CC-BY-4.0")
        self._license_edit.setText("CC-BY-4.0")
        gen_form.addRow("License", self._license_edit)

        # Library selector / display
        st = get_store()
        user_libs = [lib.name for lib in st.libraries() if not lib.readOnly]
        if not self._is_edit:
            self._lib_combo = QComboBox()
            for lib in user_libs:
                self._lib_combo.addItem(lib)
            gen_form.addRow("Library *", self._lib_combo)
        else:
            gen_form.addRow("Library",
                            QLabel(self._source.getLibrary()))

        self._name_edit = QLineEdit()
        self._name_edit.setPlaceholderText("Required")
        gen_form.addRow("Name *", self._name_edit)

        self._tags_edit = QLineEdit()
        self._tags_edit.setPlaceholderText("Comma-separated tags")
        gen_form.addRow("Tags", self._tags_edit)

        self._url_edit = QLineEdit()
        self._url_edit.setPlaceholderText("Reference URL")
        gen_form.addRow("URL", self._url_edit)

        layout.addLayout(gen_form)

        # Property tabs — sorted a-z
        tabs = QTabWidget()

        self._app_widget   = AppearanceWidget()
        self._elec_widget  = ElectricalWidget()
        self._mech_widget  = MechanicalWidget()
        self._therm_widget = ThermalWidget()

        def _scroll(w):
            area = QScrollArea()
            area.setWidgetResizable(True)
            area.setWidget(w)
            return area

        tabs.addTab(_scroll(self._app_widget),   "Appearance")
        tabs.addTab(_scroll(self._elec_widget),  "Electrical")
        tabs.addTab(_scroll(self._mech_widget),  "Mechanical")
        tabs.addTab(_scroll(self._therm_widget), "Thermal")

        layout.addWidget(tabs, stretch=1)

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
        self._author_edit.setText(mat.getAuthor())
        self._desc_edit.setText(mat.getDescription())
        self._license_edit.setText(mat.getLicense())
        self._name_edit.setText(mat.getName())
        self._tags_edit.setText(", ".join(sorted(mat.getTags())))
        self._url_edit.setText(mat.getUrl())

        self._app_widget.populate(mat.appearance())
        self._elec_widget.populate(mat.electrical())
        self._mech_widget.populate(mat.mechanical())
        self._therm_widget.populate(mat.thermal())

        if mat.isReadOnly():
            for w in (self._author_edit, self._desc_edit,
                      self._license_edit, self._name_edit,
                      self._tags_edit, self._url_edit):
                w.setReadOnly(True)

    # ------------------------------------------------------------------
    # Accept
    # ------------------------------------------------------------------

    def _on_accept(self):
        name = self._name_edit.text().strip()
        if not name:
            QMessageBox.warning(self, "NeuMaterial",
                                "Material name is required.")
            return

        if self._is_edit:
            mat = self._source
            library = mat.getLibrary()
        else:
            mat = Material(name)
            # Generate UUID for new materials
            mat.setUuid(str(_uuid_mod.uuid4()))
            mat.setVersion("2.0")
            library = self._lib_combo.currentText()

        # General fields (a-z)
        mat.setAuthor (self._author_edit.text().strip())
        mat.setDescription(self._desc_edit.text().strip())
        mat.setLicense(self._license_edit.text().strip())
        mat.setName   (name)
        mat.setUrl    (self._url_edit.text().strip())

        # Tags — parse, sort, deduplicate
        raw_tags = self._tags_edit.text()
        tags = sorted(set(
            t.strip() for t in raw_tags.split(",") if t.strip()
        ))
        mat.setTags(tags)

        # Property groups (a-z)
        self._app_widget.collect(mat.appearance())
        self._elec_widget.collect(mat.electrical())
        self._mech_widget.collect(mat.mechanical())
        self._therm_widget.collect(mat.thermal())

        # Validate
        from NeuMaterialApp import MaterialValidator
        vr = MaterialValidator.validate(mat)
        if not vr.isValid():
            QMessageBox.critical(
                self, "Validation Error",
                "Please fix the following errors:\n\n" + vr.errorSummary(),
                )
            return

        if vr.hasWarnings():
            warn_msgs = "\n".join(
                f"  [{i.field}] {i.message}"
                for i in vr.issues
                if i.severity.name == "Warning"
            )
            reply = QMessageBox.warning(
                self, "Validation Warnings",
                f"Warnings found:\n\n{warn_msgs}\n\nSave anyway?",
                QMessageBox.Yes | QMessageBox.No,
                QMessageBox.No,
                )
            if reply != QMessageBox.Yes:
                return

        self._result = (mat, library)
        self.accept()
