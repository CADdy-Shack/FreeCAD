# Init.py — loaded by FreeCAD at startup (with and without GUI).
# Registers the NeuMaterial module and imports the C++ App library.
#
# Resources layout expected under Mod/NeuMaterial/:
#   Resources/Material/Standard/   — built-in standard material library (read-only)
#   Resources/Material/Appearance/ — built-in appearance material library (read-only)
#   Resources/Models/              — reserved for future model resources

import FreeCAD

FreeCAD.addImportType("NeuMaterial YAML (*.yaml)", "NeuMaterialApp")

# Import the C++ core — side effect: initialises MaterialStore against
# Resources/Material/ and registers document types with FreeCAD.
import NeuMaterialApp  # noqa: F401  (side-effect import)

FreeCAD.Console.PrintLog("NeuMaterial: App module loaded\n")
