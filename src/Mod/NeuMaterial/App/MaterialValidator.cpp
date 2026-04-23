#include "PreCompiled.h"
#include "MaterialValidator.h"

#include <sstream>

namespace NeuMaterial::App {

// ---------------------------------------------------------------------------
// ValidationResult
// ---------------------------------------------------------------------------

bool ValidationResult::isValid() const
{
    for (const auto& issue : issues)
        if (issue.severity == ValidationIssue::Severity::Error)
            return false;
    return true;
}

bool ValidationResult::hasWarnings() const
{
    for (const auto& issue : issues)
        if (issue.severity == ValidationIssue::Severity::Warning)
            return true;
    return false;
}

std::string ValidationResult::errorSummary() const
{
    std::ostringstream oss;
    for (const auto& issue : issues) {
        if (issue.severity == ValidationIssue::Severity::Error)
            oss << "[" << issue.field << "] " << issue.message << "\n";
    }
    return oss.str();
}

// ---------------------------------------------------------------------------
// Top-level validate
// ---------------------------------------------------------------------------

ValidationResult MaterialValidator::validate(const Material& material)
{
    ValidationResult result;

    if (material.getName().empty())
        addError(result, "name", "Material name must not be empty.");

    validateMechanical(material, result);
    validateThermal   (material, result);
    validateElectrical(material, result);
    validateAppearance(material, result);

    return result;
}

// ---------------------------------------------------------------------------
// Per-category validators
// ---------------------------------------------------------------------------

void MaterialValidator::validateMechanical(const Material& mat,
                                           ValidationResult& result)
{
    const auto& m = mat.mechanical();

    if (m.density.has_value() && *m.density <= 0.0)
        addError(result, "mechanical.density",
                 "Density must be positive (kg/m³).");

    if (m.youngsModulus.has_value() && *m.youngsModulus <= 0.0)
        addError(result, "mechanical.youngs_modulus",
                 "Young's modulus must be positive (GPa).");

    if (m.poissonsRatio.has_value()) {
        if (*m.poissonsRatio <= -1.0 || *m.poissonsRatio >= 0.5)
            addError(result, "mechanical.poissons_ratio",
                     "Poisson's ratio must be in the range (-1, 0.5).");
        if (*m.poissonsRatio < 0.0)
            addWarning(result, "mechanical.poissons_ratio",
                       "Negative Poisson's ratio (auxetic material) — verify intentional.");
    }

    if (m.yieldStrength.has_value() && *m.yieldStrength <= 0.0)
        addError(result, "mechanical.yield_strength",
                 "Yield strength must be positive (MPa).");

    if (m.ultimateStrength.has_value() && *m.ultimateStrength <= 0.0)
        addError(result, "mechanical.ultimate_strength",
                 "Ultimate strength must be positive (MPa).");

    if (m.yieldStrength.has_value() && m.ultimateStrength.has_value()
        && *m.yieldStrength > *m.ultimateStrength)
        addWarning(result, "mechanical.yield_strength",
                   "Yield strength exceeds ultimate strength — verify values.");
}

void MaterialValidator::validateThermal(const Material& mat,
                                        ValidationResult& result)
{
    const auto& t = mat.thermal();

    if (t.thermalConductivity.has_value() && *t.thermalConductivity < 0.0)
        addError(result, "thermal.thermal_conductivity",
                 "Thermal conductivity must be non-negative (W/m·K).");

    if (t.thermalExpansion.has_value() && *t.thermalExpansion < 0.0)
        addWarning(result, "thermal.thermal_expansion",
                   "Negative thermal expansion coefficient — verify intentional.");

    if (t.specificHeat.has_value() && *t.specificHeat <= 0.0)
        addError(result, "thermal.specific_heat",
                 "Specific heat must be positive (J/kg·K).");

    if (t.meltingPoint.has_value() && *t.meltingPoint < -273.15)
        addError(result, "thermal.melting_point",
                 "Melting point cannot be below absolute zero (−273.15 °C).");
}

void MaterialValidator::validateElectrical(const Material& mat,
                                           ValidationResult& result)
{
    const auto& e = mat.electrical();

    if (e.electricalConductivity.has_value() && *e.electricalConductivity < 0.0)
        addError(result, "electrical.electrical_conductivity",
                 "Electrical conductivity must be non-negative (S/m).");

    if (e.resistivity.has_value() && *e.resistivity < 0.0)
        addError(result, "electrical.resistivity",
                 "Resistivity must be non-negative (Ω·m).");

    if (e.permittivity.has_value() && *e.permittivity < 0.0)
        addError(result, "electrical.permittivity",
                 "Permittivity must be non-negative (F/m).");

    // Consistency check: conductivity and resistivity should be reciprocals
    if (e.electricalConductivity.has_value() && e.resistivity.has_value()
        && *e.electricalConductivity > 0.0 && *e.resistivity > 0.0) {
        double product = *e.electricalConductivity * *e.resistivity;
        if (std::abs(product - 1.0) > 0.05)   // 5 % tolerance
            addWarning(result, "electrical.resistivity",
                       "Conductivity × resistivity deviates from 1.0 — check consistency.");
    }
}

void MaterialValidator::validateAppearance(const Material& mat,
                                           ValidationResult& result)
{
    const auto& a = mat.appearance();

    auto inRange01 = [](float v) { return v >= 0.0f && v <= 1.0f; };

    for (int i = 0; i < 4; ++i) {
        if (!inRange01(a.color[i])) {
            addError(result, "appearance.color",
                     "Color RGBA components must be in [0, 1].");
            break;
        }
    }

    if (!inRange01(a.roughness))
        addError(result, "appearance.roughness", "Roughness must be in [0, 1].");

    if (!inRange01(a.metallic))
        addError(result, "appearance.metallic", "Metallic must be in [0, 1].");

    if (!inRange01(a.opacity))
        addError(result, "appearance.opacity", "Opacity must be in [0, 1].");
}

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

void MaterialValidator::addError(ValidationResult& r,
                                 const std::string& field,
                                 const std::string& message)
{
    r.issues.push_back({ValidationIssue::Severity::Error, field, message});
}

void MaterialValidator::addWarning(ValidationResult& r,
                                   const std::string& field,
                                   const std::string& message)
{
    r.issues.push_back({ValidationIssue::Severity::Warning, field, message});
}

} // namespace NeuMaterial::App
