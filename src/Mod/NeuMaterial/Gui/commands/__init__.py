"""
NeuMaterial commands package.
Importing this package registers all commands with FreeCADGui.
"""

import FreeCADGui

from .cmd_browse_library    import BrowseLibraryCommand
from .cmd_new_material      import NewMaterialCommand
from .cmd_edit_material     import EditMaterialCommand
from .cmd_duplicate_material import DuplicateMaterialCommand
from .cmd_delete_material   import DeleteMaterialCommand

FreeCADGui.addCommand("NeuMaterial_BrowseLibrary",    BrowseLibraryCommand())
FreeCADGui.addCommand("NeuMaterial_NewMaterial",      NewMaterialCommand())
FreeCADGui.addCommand("NeuMaterial_EditMaterial",     EditMaterialCommand())
FreeCADGui.addCommand("NeuMaterial_DuplicateMaterial", DuplicateMaterialCommand())
FreeCADGui.addCommand("NeuMaterial_DeleteMaterial",   DeleteMaterialCommand())
