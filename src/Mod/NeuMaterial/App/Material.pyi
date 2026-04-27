# SPDX-FileCopyrightText: 2026 Michael Cummings <mgcummings@yahoo.com>
# SPDX-License-Identifier: CC-BY-4.0

from __future__ import annotations

from Base.Metadata import export, no_args
from Base.BaseClass import BaseClass
from typing import Final, List, Optional


@export(
    Include="Mod/NeuMaterial/App/Material.h",
    Namespace="NeuMaterial::App",
    Constructor=True,
    Delete=True,
)
class Material(BaseClass):
    """
    A NeuMaterial material — physical, thermal, electrical, and appearance
    properties organized by declared model UUIDs.

    Author: Michael Cummings (mgcummings@yahoo.com)
    Licence: CC-BY-4.0
    """

    # ------------------------------------------------------------------
    # General metadata (read-write)
    # ------------------------------------------------------------------

    Author: str = ...
    """Author of the material definition."""

    Description: str = ...
    """Human-readable description of the material."""

    License: str = ...
    """License identifier, e.g. 'CC-BY-4.0'."""

    Name: str = ...
    """Material display name."""

    Tags: List[str] = ...
    """Sorted list of searchable tags."""

    URL: str = ...
    """Reference URL for this material."""

    UUID: str = ...
    """Unique material identifier (UUID v4)."""

    Version: str = ...
    """Format version string, e.g. '2.0'."""

    # ------------------------------------------------------------------
    # Store-level metadata (read-only from Python)
    # ------------------------------------------------------------------

    Category: Final[str] = ...
    """Category path derived from the material file path, e.g. 'Metal/Steel'."""

    Library: Final[str] = ...
    """Name of the library this material belongs to."""

    ReadOnly: Final[bool] = ...
    """True if this material belongs to a built-in read-only library."""

    # ------------------------------------------------------------------
    # Model references
    # ------------------------------------------------------------------

    PhysicalModels: Final[List[str]] = ...
    """
    UUIDs of physical models declared by this material
    (e.g. LinearElastic, Thermal, Electrical).
    Returns all modelUuids that are not appearance models.
    """

    AppearanceModels: Final[List[str]] = ...
    """
    UUIDs of appearance models declared by this material.
    Currently returns model UUIDs identified as appearance-related.
    Future work: split stored separately from physical models.
    """

    # ------------------------------------------------------------------
    # Mechanical properties (read-write, optional — None if not set)
    # ------------------------------------------------------------------

    Density: Optional[float] = ...
    """Density in kg/m³."""

    PoissonsRatio: Optional[float] = ...
    """Poisson's ratio, dimensionless, in range (-1.0, 0.5)."""

    UltimateStrength: Optional[float] = ...
    """Ultimate tensile strength in MPa."""

    YieldStrength: Optional[float] = ...
    """Yield strength in MPa."""

    YoungsModulus: Optional[float] = ...
    """Young's modulus in GPa."""

    # ------------------------------------------------------------------
    # Thermal properties (read-write, optional)
    # ------------------------------------------------------------------

    MeltingPoint: Optional[float] = ...
    """Melting point in degrees Celsius."""

    SpecificHeat: Optional[float] = ...
    """Specific heat capacity in J/kg·K."""

    ThermalConductivity: Optional[float] = ...
    """Thermal conductivity in W/m·K."""

    ThermalExpansion: Optional[float] = ...
    """Linear thermal expansion coefficient in µm/m·K."""

    # ------------------------------------------------------------------
    # Electrical properties (read-write, optional)
    # ------------------------------------------------------------------

    ElectricalConductivity: Optional[float] = ...
    """Electrical conductivity in S/m."""

    Permittivity: Optional[float] = ...
    """Absolute permittivity in F/m."""

    Resistivity: Optional[float] = ...
    """Electrical resistivity in Ohm·m."""

    # ------------------------------------------------------------------
    # Appearance properties — Phong / BasicRendering model (read-write)
    # ------------------------------------------------------------------

    AmbientColor: List[float] = ...
    """Color of the surface under indirect uniform lighting as [R, G, B, A].
    Accepts 3-component [R, G, B] list; alpha defaults to 1.0."""

    DiffuseColor: List[float] = ...
    """Base color of the surface when illuminated by light as [R, G, B, A].
    Accepts 3-component [R, G, B] list; alpha defaults to 1.0."""

    EmissiveColor: List[float] = ...
    """Color the surface appears to emit as [R, G, B, A].
    Set to black [0,0,0,1] for no emission."""

    Shininess: float = ...
    """Size and sharpness of specular highlights in [0.0, 1.0].
    Higher values produce smaller, sharper highlights."""

    SpecularColor: List[float] = ...
    """Color of mirror-like highlights as [R, G, B, A].
    Accepts 3-component [R, G, B] list; alpha defaults to 1.0."""

    Transparency: float = ...
    """Transparency in [0.0, 1.0]: 0.0 = opaque, 1.0 = fully transparent."""
    
    # ------------------------------------------------------------------
    # Methods
    # ------------------------------------------------------------------

    def isValid(self) -> bool:
        """Returns True if the material has a non-empty Name and UUID."""
        ...

    def addModelUuid(self, uuid: str) -> None:
        """Append a model UUID to the material's model list."""
        ...

    def clearModelUuids(self) -> None:
        """Remove all model UUID references from this material."""
        ...

    def hasModelUuid(self, uuid: str) -> bool:
        """Return True if the given UUID is in the material's model list."""
        ...

    @no_args
    def validate(self) -> object:
        """
        Validate this material against its declared models using the
        global ModelStore. Returns a ValidationResult object.
        """
        ...
