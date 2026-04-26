# SPDX-FileCopyrightText: 2026 Michael Cummings <mgcummings@yahoo.com>
# SPDX-License-Identifier: CC-BY-4.0

from __future__ import annotations

from Base.Metadata import export, no_args
from Base.BaseClass import BaseClass
from typing import Final, List, Dict


@export(
    Include="Mod/NeuMaterial/App/ModelStore.h",
    Namespace="NeuMaterial::App",
    Constructor=True,
)
class ModelStore(BaseClass):
    """
    Registry for all NeuMaterial model definitions loaded from
    Resources/Models/. Access the process-wide singleton via
    NeuMaterialApp.modelStore().

    Author: Michael Cummings (mgcummings@yahoo.com)
    Licence: CC-BY-4.0
    """

    Models: Final[Dict] = ...
    """Dict of all loaded models keyed by UUID."""

    def getModel(self, uuid: str) -> object:
        """
        Return the MaterialModel with the given UUID,
        or raise LookupError if not found.
        """
        ...

    def getModelByName(self, name: str) -> object:
        """
        Return the MaterialModel with the given name (case-sensitive),
        or raise LookupError if not found.
        """
        ...

    def resolvedProperties(self, uuid: str) -> Dict:
        """
        Return a flat dict of all PropertyDefinitions for the model UUID,
        resolving inheritance recursively. Each value is a dict with keys:
        'name', 'type', 'units', 'url', 'description', 'minimum', 'maximum'.
        """
        ...

    def resolvedPropertiesForModels(self, uuids: List[str]) -> Dict:
        """
        Return the union of resolved properties across all given model UUIDs.
        Later entries win on key conflicts.
        """
        ...

    @no_args
    def allModels(self) -> List:
        """Return all loaded models as a list, sorted by name."""
        ...
