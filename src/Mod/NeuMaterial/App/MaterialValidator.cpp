#include "MaterialValidator.h"

namespace NeuMaterial::App {

// ---------------------------------------------------------------------------
// ValidationResult
// ---------------------------------------------------------------------------

bool ValidationResult::isValid() const
{
    return !hasErrors();
}

bool ValidationResult::hasErrors() const
{
    for (const auto& issue : issues)
        if (issue.severity == ValidationIssue::Severity::Error)
            return true;
    return false;
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
    for (const auto& issue : issues)
        if (issue.severity == ValidationIssue::Severity::Error)
            oss << "[" << issue.field << "] " << issue.message << "\n";
    return oss.str();
}

std::string ValidationResult::warningSummary() const
{
    std::ostringstream oss;
    for (const auto& issue : issues)
        if (issue.severity == ValidationIssue::Severity::Warning)
            oss << "[" << issue.field << "] " << issue.message << "\n";
    return oss.str();
}

// ---------------------------------------------------------------------------
// Top-level validate
// ---------------------------------------------------------------------------

ValidationResult MaterialValidator::validate(const Material& material)
{
    ValidationResult result;

    // Identity checks
    if (material.getName().empty())
        addError(result, "General.Name", "Material name must not be empty.");
    if (material.getUuid().empty())
        addError(result, "General.UUID", "Material UUID must not be empty.");

    // Per-model-group validation (a-z)
    validateAppearance   (material, result);
    validateElectrical   (material, result);
    validateLinearElastic(material, result);
    validateThermal      (material, result);

    return result;
}

// ---------------------------------------------------------------------------
// Per-model-group validators
// ---------------------------------------------------------------------------

void MaterialValidator::validateAppearance(const Material& mat,
                                           ValidationResult& r)
{
    const auto& a = mat.appearance();

    auto inRange = [](float v) { return v >= 0.0f && v <= 1.0f; };

    for (int i = 0; i < 4; ++i) {
        if (!inRange(a.color[i])) {
            addError(r, "RenderAppearance.Color",
                     "Color RGBA components must each be in [0.0, 1.0].");
            break;
        }
    }
    if (!inRange(a.metallic))
        addError(r, "RenderAppearance.Metallic",
                 "Metallic must be in [0.0, 1.0].");
    if (!inRange(a.opacity))
        addError(r, "RenderAppearance.Opacity",
                 "Opacity must be in [0.0, 1.0].");
    if (!inRange(a.roughness))
        addError(r, "RenderAppearance.Roughness",
                 "Roughness must be in [0.0, 1.0].");
}

void MaterialValidator::validateElectrical(const Material& mat,
                                           ValidationResult& r)
{
    const auto& e = mat.electrical();

    checkBounds("Electrical", "ElectricalConductivity",
                e.electricalConductivity, r);
    checkBounds("Electrical", "Permittivity",
                e.permittivity, r);
    checkBounds("Electrical", "Resistivity",
                e.resistivity, r);

    // Cross-field: conductivity and resistivity should be reciprocals
    if (e.electricalConductivity.has_value() && e.resistivity.has_value()
        && *e.electricalConductivity > 0.0 && *e.resistivity > 0.0) {
        double product = *e.electricalConductivity * *e.resistivity;
        if (std::abs(product - 1.0) > 0.05)
            addWarning(r, "Electrical.Resistivity",
                       "ElectricalConductivity x Resistivity deviates from "
                       "1.0 by more than 5% — check consistency.");
    }
}

void MaterialValidator::validateLinearElastic(const Material& mat,
                                              ValidationResult& r)
{
    const auto& m = mat.mechanical();

    checkBounds("LinearElastic", "Density",          m.density,          r);
    checkBounds("LinearElastic", "PoissonsRatio",    m.poissonsRatio,    r);
    checkBounds("LinearElastic", "UltimateStrength", m.ultimateStrength, r);
    checkBounds("LinearElastic", "YieldStrength",    m.yieldStrength,    r);
    checkBounds("LinearElastic", "YoungsModulus",    m.youngsModulus,    r);

    // Cross-field: yield must not exceed ultimate
    if (m.yieldStrength.has_value() && m.ultimateStrength.has_value()
        && *m.yieldStrength > *m.ultimateStrength)
        addWarning(r, "LinearElastic.YieldStrength",
                   "YieldStrength exceeds UltimateStrength — verify values.");

    // Auxetic material warning
    if (m.poissonsRatio.has_value() && *m.poissonsRatio < 0.0)
        addWarning(r, "LinearElastic.PoissonsRatio",
                   "Negative Poisson's ratio (auxetic material) — "
                   "verify this is intentional.");
}

void MaterialValidator::validateThermal(const Material& mat,
                                        ValidationResult& r)
{
    const auto& t = mat.thermal();

    checkBounds("Thermal", "MeltingPoint",        t.meltingPoint,        r);
    checkBounds("Thermal", "SpecificHeat",        t.specificHeat,        r);
    checkBounds("Thermal", "ThermalConductivity", t.thermalConductivity, r);
    checkBounds("Thermal", "ThermalExpansion",    t.thermalExpansion,    r);

    // Negative thermal expansion is physically possible (auxetic) but unusual
    if (t.thermalExpansion.has_value() && *t.thermalExpansion < 0.0)
        addWarning(r, "Thermal.ThermalExpansion",
                   "Negative thermal expansion coefficient — "
                   "verify this is intentional.");
}

// ---------------------------------------------------------------------------
// Model-driven range check
// ---------------------------------------------------------------------------

void MaterialValidator::checkBounds(const std::string& modelKey,
                                    const std::string& propKey,
                                    std::optional<double> value,
                                    ValidationResult& r)
{
    if (!value.has_value()) return;

    const std::string field = modelKey + "." + propKey;
    const ModelStore& store = modelStore();

    // Resolve property definition via ModelStore
    // Try by model name first, then fall back to resolved properties
    const MaterialModel* model = store.findByName(modelKey);
    if (!model) return;  // unknown model — skip bounds check

    // For composite models resolve through inheritance
    auto props = store.resolvedProperties(model->getUuid());
    auto it = props.find(propKey);
    if (it == props.end()) return;  // property not in model

    const PropertyDefinition& def = it->second;

    if (def.minimum.has_value() && *value < *def.minimum) {
        std::ostringstream msg;
        msg << propKey << " value " << *value
            << " is below minimum " << *def.minimum
            << " (" << def.units << ").";
        addError(r, field, msg.str());
    }

    if (def.maximum.has_value() && *value > *def.maximum) {
        std::ostringstream msg;
        msg << propKey << " value " << *value
            << " exceeds maximum " << *def.maximum
            << " (" << def.units << ").";
        addError(r, field, msg.str());
    }
}

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

void MaterialValidator::addError(ValidationResult& r,
                                 const std::string& field,
                                 const std::string& message)
{
    r.issues.push_back({field, message, ValidationIssue::Severity::Error});
}

void MaterialValidator::addWarning(ValidationResult& r,
                                   const std::string& field,
                                   const std::string& message)
{
    r.issues.push_back({field, message, ValidationIssue::Severity::Warning});
}

} // namespace NeuMaterial::App
