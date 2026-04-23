#pragma once

extern "C" void initNeuMaterialGui();

namespace NeuMaterial::Gui {

/// Called once when the Gui module is first imported.
/// Registers the workbench type and GUI resources.
void initializeGui();

} // namespace NeuMaterial::Gui
