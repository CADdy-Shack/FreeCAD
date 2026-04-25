"""
Reusable input widgets for material property groups.
Each widget exposes populate(props) / collect(props) methods.
Row labels and field order match the Model DisplayName and are sorted a-z.
"""

from PySide6.QtGui     import QColor
from PySide6.QtWidgets import (
    QColorDialog, QFormLayout, QDoubleSpinBox,
    QFrame, QGroupBox, QHBoxLayout, QLabel,
    QLineEdit, QPushButton, QWidget,
)


# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------

def _spin(min_val, max_val, decimals=4, suffix=""):
    """Create a QDoubleSpinBox with a sentinel null value at min_val."""
    sb = QDoubleSpinBox()
    sb.setRange(min_val, max_val)
    sb.setDecimals(decimals)
    sb.setSuffix(f"  {suffix}" if suffix else "")
    sb.setSpecialValueText("\u2014")  # em-dash shown when at null sentinel
    sb.setValue(min_val)
    return sb


def _opt_from_spin(spin, null_sentinel):
    """Return None if spin is at the null sentinel, else its value."""
    v = spin.value()
    return None if abs(v - null_sentinel) < 1e-12 else v


def _set_spin(spin, value, null_sentinel):
    spin.setValue(null_sentinel if value is None else value)


# ---------------------------------------------------------------------------
# AppearanceWidget
# ---------------------------------------------------------------------------

class AppearanceWidget(QGroupBox):
    """PBR appearance properties — Color, Metallic, Opacity, Roughness, TexturePath."""

    def __init__(self, parent=None):
        super().__init__("Appearance", parent)
        form = QFormLayout(self)

        # Color swatch + pick button (sorted first as 'Color')
        color_row = QHBoxLayout()
        self._color_swatch = QLabel()
        self._color_swatch.setFixedSize(32, 18)
        self._color_swatch.setFrameShape(QFrame.Box)
        self._color = QColor(204, 204, 204, 255)
        self._refresh_swatch()
        pick_btn = QPushButton("Pick\u2026")
        pick_btn.setFixedWidth(56)
        pick_btn.clicked.connect(self._pick_color)
        color_row.addWidget(self._color_swatch)
        color_row.addWidget(pick_btn)
        color_row.addStretch()
        form.addRow("Color (RGBA)", color_row)

        # Sorted a-z after Color
        self._metallic  = _spin(0.0, 1.0, 3)
        self._opacity   = _spin(0.0, 1.0, 3)
        self._roughness = _spin(0.0, 1.0, 3)
        self._texture   = QLineEdit()
        self._texture.setPlaceholderText("(none)")

        self._metallic.setValue(0.0)
        self._opacity.setValue(1.0)
        self._roughness.setValue(0.5)

        form.addRow("Metallic",      self._metallic)
        form.addRow("Opacity",       self._opacity)
        form.addRow("Roughness",     self._roughness)
        form.addRow("Texture path",  self._texture)

    def populate(self, props):
        r, g, b, a = (int(c * 255) for c in props.color)
        self._color = QColor(r, g, b, a)
        self._refresh_swatch()
        self._metallic.setValue(props.metallic)
        self._opacity.setValue(props.opacity)
        self._roughness.setValue(props.roughness)
        self._texture.setText(props.texturePath or "")

    def collect(self, props):
        props.color = [
            self._color.redF(),
            self._color.greenF(),
            self._color.blueF(),
            self._color.alphaF(),
        ]
        props.metallic    = self._metallic.value()
        props.opacity     = self._opacity.value()
        props.roughness   = self._roughness.value()
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
            f"background-color: "
            f"{self._color.name(QColor.NameFormat.HexArgb)};"
        )


# ---------------------------------------------------------------------------
# ElectricalWidget
# ---------------------------------------------------------------------------

class ElectricalWidget(QGroupBox):
    """Electrical properties — ElectricalConductivity, Permittivity, Resistivity."""

    _NULL = -1.0

    def __init__(self, parent=None):
        super().__init__("Electrical", parent)
        form = QFormLayout(self)

        # Sorted a-z
        self._conductivity = _spin(self._NULL, 1e9,  3, "S/m")
        self._permittivity = _spin(self._NULL, 1e-6, 6, "F/m")
        self._resistivity  = _spin(self._NULL, 1e15, 3, "\u03a9\u00b7m")

        form.addRow("Electrical conductivity", self._conductivity)
        form.addRow("Permittivity",            self._permittivity)
        form.addRow("Resistivity",             self._resistivity)

    def populate(self, props):
        _set_spin(self._conductivity, props.electricalConductivity, self._NULL)
        _set_spin(self._permittivity, props.permittivity,           self._NULL)
        _set_spin(self._resistivity,  props.resistivity,            self._NULL)

    def collect(self, props):
        props.electricalConductivity = _opt_from_spin(self._conductivity, self._NULL)
        props.permittivity           = _opt_from_spin(self._permittivity, self._NULL)
        props.resistivity            = _opt_from_spin(self._resistivity,  self._NULL)


# ---------------------------------------------------------------------------
# MechanicalWidget
# ---------------------------------------------------------------------------

class MechanicalWidget(QGroupBox):
    """Mechanical properties — Density, PoissonsRatio, UltimateStrength,
    YieldStrength, YoungsModulus."""

    _NULL = -1.0

    def __init__(self, parent=None):
        super().__init__("Mechanical", parent)
        form = QFormLayout(self)

        # Sorted a-z matching MechanicalProperties struct
        self._density          = _spin(self._NULL, 30000, 1, "kg/m\u00b3")
        self._poissons_ratio   = _spin(self._NULL, 0.5,   4)
        self._ultimate_strength = _spin(self._NULL, 5000, 1, "MPa")
        self._yield_strength   = _spin(self._NULL, 5000,  1, "MPa")
        self._youngs_modulus   = _spin(self._NULL, 10000, 3, "GPa")

        form.addRow("Density",            self._density)
        form.addRow("Poisson's ratio",    self._poissons_ratio)
        form.addRow("Ultimate strength",  self._ultimate_strength)
        form.addRow("Yield strength",     self._yield_strength)
        form.addRow("Young's modulus",    self._youngs_modulus)

    def populate(self, props):
        _set_spin(self._density,           props.density,          self._NULL)
        _set_spin(self._poissons_ratio,    props.poissonsRatio,    self._NULL)
        _set_spin(self._ultimate_strength, props.ultimateStrength, self._NULL)
        _set_spin(self._yield_strength,    props.yieldStrength,    self._NULL)
        _set_spin(self._youngs_modulus,    props.youngsModulus,    self._NULL)

    def collect(self, props):
        props.density          = _opt_from_spin(self._density,           self._NULL)
        props.poissonsRatio    = _opt_from_spin(self._poissons_ratio,    self._NULL)
        props.ultimateStrength = _opt_from_spin(self._ultimate_strength, self._NULL)
        props.yieldStrength    = _opt_from_spin(self._yield_strength,    self._NULL)
        props.youngsModulus    = _opt_from_spin(self._youngs_modulus,    self._NULL)


# ---------------------------------------------------------------------------
# ThermalWidget
# ---------------------------------------------------------------------------

class ThermalWidget(QGroupBox):
    """Thermal properties — MeltingPoint, SpecificHeat,
    ThermalConductivity, ThermalExpansion."""

    _NULL = -1.0

    def __init__(self, parent=None):
        super().__init__("Thermal", parent)
        form = QFormLayout(self)

        # Sorted a-z matching ThermalProperties struct
        # MeltingPoint uses -273.15 as its null sentinel (absolute zero)
        self._melting_point        = _spin(-273.15, 5000,  1, "\u00b0C")
        self._specific_heat        = _spin(self._NULL, 10000, 1, "J/kg\u00b7K")
        self._thermal_conductivity = _spin(self._NULL, 5000,  3, "W/m\u00b7K")
        self._thermal_expansion    = _spin(self._NULL, 500,   3, "\u03bcm/m\u00b7K")

        form.addRow("Melting point",         self._melting_point)
        form.addRow("Specific heat",         self._specific_heat)
        form.addRow("Thermal conductivity",  self._thermal_conductivity)
        form.addRow("Thermal expansion",     self._thermal_expansion)

    def populate(self, props):
        _set_spin(self._melting_point,        props.meltingPoint,        -273.15)
        _set_spin(self._specific_heat,        props.specificHeat,        self._NULL)
        _set_spin(self._thermal_conductivity, props.thermalConductivity, self._NULL)
        _set_spin(self._thermal_expansion,    props.thermalExpansion,    self._NULL)

    def collect(self, props):
        props.meltingPoint        = _opt_from_spin(self._melting_point,        -273.15)
        props.specificHeat        = _opt_from_spin(self._specific_heat,        self._NULL)
        props.thermalConductivity = _opt_from_spin(self._thermal_conductivity, self._NULL)
        props.thermalExpansion    = _opt_from_spin(self._thermal_expansion,    self._NULL)
