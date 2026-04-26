# SPDX-FileCopyrightText: 2026 Michael Cummings <mgcummings@yahoo.com>
# SPDX-License-Identifier: CC-BY-4.0

from __future__ import annotations

from Base.Metadata import export, no_args
from Base.BaseClass import BaseClass
from typing import Final, List


@export(
    Include="Mod/NeuMaterial/App/MaterialStore.h",
    Namespace="NeuMaterial::App",
    Constructor=True,
)
class MaterialStore(BaseClass):
    """
    Central registry for all NeuMaterial libraries and materials.
    Access the process-wide singleton via NeuMaterialApp.materialStore().

    Author: Michael Cummings (mgcummings@yahoo.com)
    Licence: CC-BY-4.0
    """

    Libraries: Final[List] = ...
    """
    List of library descriptors. Each entry is a dict with keys:
    'name' (str), 'readOnly' (bool), 'rootPath' (str).
    """

    Materials: Final[dict] = ...
    """Dict of all materials across all libraries, keyed by UUID."""

    def initialize(self, resourcesPath: str, userLibrariesRoot: str = "") -> None:
        """
        Initialize the store from the given Resources/ directory.
        Called automatically by NeuMaterialApp at startup.
        """
        ...

    def getMaterial(self, uuid: str) -> object:
        """
        Return the Material with the given UUID, or raise LookupError
        if not found.
        """
        ...

    def getMaterialByName(self, name: str) -> object:
        """
        Return the first Material whose name matches (case-insensitive),
        or raise LookupError if not found.
        """
        ...

    def getMaterialsInLibrary(self, libraryName: str) -> List:
        """Return a list of all Materials in the named library."""
        ...

    def addMaterial(self, material: object, libraryName: str) -> object:
        """
        Add a new material to the named user library, persist it to disk,
        and return the stored Material. Raises ValueError if the library
        is read-only or not found.
        """
        ...

    def updateMaterial(self, material: object) -> None:
        """
        Persist changes to an existing user-library material.
        Raises ValueError if the material is read-only or not found.
        """
        ...

    def deleteMaterial(self, uuid: str) -> None:
        """
        Delete a user-library material from memory and disk.
        Raises ValueError if the material is read-only.
        """
        ...

    def duplicateMaterial(self, sourceUuid: str, targetLibrary: str,
                          newName: str = "") -> object:
        """
        Copy any material (including read-only) into a user library,
        assigning a new UUID. Returns the new Material.
        """
        ...

    def createUserLibrary(self, name: str) -> bool:
        """
        Create a new empty user library. Returns False if a library
        with that name already exists.
        """
        ...

    def removeUserLibrary(self, name: str) -> bool:
        """
        Remove a user library and all its materials from the disk.
        Returns False if the library is built-in or not found.
        """
        ...

    @no_args
    def reload(self) -> None:
        """Reload all materials from the disk."""
        ...
