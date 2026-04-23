"""
Reusable input widgets for material property groups.
Each widget exposes populate(data) / collect() → data methods.
"""

from PySide6.QtCore    import Qt
from PySide6.QtGui     import QColor, QDoubleValidator
from PySide6.QtWidgets import (
    QWidget, QFormLayout, QLineEdit, QDoubleSpinBox,
    QGroupBox, QVBoxLayout, QHBoxLayout, QPushButton,
    QLabel, QColorDialog, QFrame,
)


# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------

def _spin(min_val, max_val, decimals=4, suffix=""):
    sb = QDoubleSpinBox()
    sb.setRange(min_val, max_val)
    sb.setDecimals(decimals)
    sb.setSuffix(f"  {suffix}" if suffix else "")
    sb.setSpecialValueText("—")    # shown when value == minimum (used as "null")
    sb.setValue(min_val)           # default = "null" sentinel
    return sb


def _optional_from_spin(spin, null_sentinel):
    """Return None if spin is at the null sentinel, else the value."""
    v = spin.value()
    return None if abs(v - null_sentinel) < 1e-12 else v


def _populate_spin(spin, value, null_sentinel):
    spin.setValue(null_sentinel if value is None else value)


# ---------------------------------------------------------------------------
# MechanicalWidget
# ---------------------------------------------------------------------------

class MechanicalWidget(QGroupBox):
    _NULL = -1.0   # sentinel: spin at this value → property is null

    def __init__(self, parent=None):
        super().__init__("Mechanical", parent)
        form = QFormLayout(self)

        self._density    = _spin(self._NULL, 30000, 1, "kg/m³")
        self._youngs     = _spin(self._NULL, 10000, 3, "GPa")
        self._poissons   = _spin(self._NULL, 0.5,   4, "")
        self._yield_str  = _spin(self._NULL, 5000,  1, "MPa")
        self._ultimate   = _spin(self._NULL, 5000,  1, "MPa")

        form.addRow("Density",           self._density)
        form.addRow("Young's modulus",   self._youngs)
        form.addRow("Poisson's ratio",   self._poissons)
        form.addRow("Yield strength",    self._yield_str)
        form.addRow("Ultimate strength", self._ultimate)

    def populate(self, props):
        _populate_spin(self._density,   props.density,          self._NULL)
        _populate_spin(self._youngs,    props.youngsModulus,     self._NULL)
        _populate_spin(self._poissons,  props.poissonsRatio,     self._NULL)
        _populate_spin(self._yield_str, props.yieldStrength,     self._NULL)
        _populate_spin(self._ultimate,  props.ultimateStrength,  self._NULL)

    def collect(self, props):
        props.density          = _optional_from_spin(self._density,   self._NULL)
        props.youngsModulus    = _optional_from_spin(self._youngs,    self._NULL)
        props.poissonsRatio    = _optional_from_spin(self._poissons,  self._NULL)
        props.yieldStrength    = _optional_from_spin(self._yield_str, self._NULL)
        props.ultimateStrength = _optional_from_spin(self._ultimate,  self._NULL)


# ---------------------------------------------------------------------------
# ThermalWidget
# ---------------------------------------------------------------------------

class ThermalWidget(QGroupBox):
    _NULL = -1.0

    def __init__(self, parent=None):
        super().__init__("Thermal", parent)
        form = QFormLayout(self)

        self._conductivity = _spin(self._NULL, 5000,   3, "W/m·K")
        self._expansion    = _spin(self._NULL, 500,    3, "µm/m·K")
        self._specific_heat = _spin(self._NULL, 10000, 1, "J/kg·K")
        self._melting      = _spin(-273.15,    5000,   1, "°C")

        form.addRow("Thermal conductivity",  self._conductivity)
        form.addRow("Thermal expansion",     self._expansion)
        form.addRow("Specific heat",         self._specific_heat)
        form.addRow("Melting point",         self._melting)

    def populate(self, props):
        _populate_spin(self._conductivity,  props.thermalConductivity, self._NULL)
        _populate_spin(self._expansion,     props.thermalExpansion,    self._NULL)
        _populate_spin(self._specific_heat, props.specificHeat,        self._NULL)
        _populate_spin(self._melting,       props.meltingPoint,        -273.15)

    def collect(self, props):
        props.thermalConductivity = _optional_from_spin(self._conductivity,  self._NULL)
        props.thermalExpansion    = _optional_from_spin(self._expansion,     self._NULL)
        props.specificHeat        = _optional_from_spin(self._specific_heat, self._NULL)
        props.meltingPoint        = _optional_from_spin(self._melting,       -273.15)


# ---------------------------------------------------------------------------
# ElectricalWidget
# ---------------------------------------------------------------------------

class ElectricalWidget(QGroupBox):
    _NULL = -1.0

    def __init__(self, parent=None):
        super().__init__("Electrical", parent)
        form = QFormLayout(self)

        self._conductivity = _spin(self._NULL, 1e9,  3, "S/m")
        self._permittivity = _spin(self._NULL, 1e-6, 6, "F/m")
        self._resistivity  = _spin(self._NULL, 1e15, 3, "Ω·m")

        form.addRow("Electrical conductivity", self._conductivity)
        form.addRow("Permittivity",            self._permittivity)
        form.addRow("Resistivity",             self._resistivity)

    def populate(self, props):
        _populate_spin(self._conductivity, props.electricalConductivity, self._NULL)
        _populate_spin(self._permittivity, props.permittivity,           self._NULL)
        _populate_spin(self._resistivity,  props.resistivity,            self._NULL)

    def collect(self, props):
        props.electricalConductivity = _optional_from_spin(self._conductivity, self._NULL)
        props.permittivity           = _optional_from_spin(self._permittivity, self._NULL)
        props.resistivity            = _optional_from_spin(self._resistivity,  self._NULL)


# ---------------------------------------------------------------------------
# AppearanceWidget
# ---------------------------------------------------------------------------

class AppearanceWidget(QGroupBox):
    def __init__(self, parent=None):
        super().__init__("Appearance", parent)
        form = QFormLayout(self)

        # Color swatch + button
        color_row = QHBoxLayout()
        self._color_swatch = QLabel()
        self._color_swatch.setFixedSize(32, 18)
        self._color_swatch.setFrameShape(QFrame.Box)
        self._color = QColor(204, 204, 204, 255)
        self._refresh_swatch()
        pick_btn = QPushButton("Pick…")
        pick_btn.setFixedWidth(56)
        pick_btn.clicked.connect(self._pick_color)
        color_row.addWidget(self._color_swatch)
        color_row.addWidget(pick_btn)
        color_row.addStretch()
        form.addRow("Color (RGBA)", color_row)

        self._roughness = _spin(0.0, 1.0, 3, "")
        self._metallic  = _spin(0.0, 1.0, 3, "")
        self._opacity   = _spin(0.0, 1.0, 3, "")

        self._roughness.setValue(0.5)
        self._metallic.setValue(0.0)
        self._opacity.setValue(1.0)

        self._texture = QLineEdit()
        self._texture.setPlaceholderText("(none)")

        form.addRow("Roughness",    self._roughness)
        form.addRow("Metallic",     self._metallic)
        form.addRow("Opacity",      self._opacity)
        form.addRow("Texture path", self._texture)

    def populate(self, props):
        r, g, b, a = (int(c * 255) for c in props.color)
        self._color = QColor(r, g, b, a)
        self._refresh_swatch()
        self._roughness.setValue(props.roughness)
        self._metallic.setValue(props.metallic)
        self._opacity.setValue(props.opacity)
        self._texture.setText(props.texturePath or "")

    def collect(self, props):
        props.color       = [
            self._color.redF(),
            self._color.greenF(),
            self._color.blueF(),
            self._color.alphaF(),
        ]
        props.roughness   = self._roughness.value()
        props.metallic    = self._metallic.value()
        props.opacity     = self._opacity.value()
        props.texturePath = self._texture.text().strip()

    def _pick_color(self):
        chosen = QColorDialog.getColor(
            self._color, self, "Pick Material Color",
            QColorDialog.ShowAlphaChannel,
        )
        if chosen.isValid():
            self._color = chosen
            self._refresh_swatch()

    def _refresh_swatch(self):
        self._color_swatch.setStyleSheet(
            f"background-color: {self._color.name(QColor.NameFormat.HexArgb)};"
        )
