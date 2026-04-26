#pragma once

#include "PreCompiled.h"
#include "Material.h"

namespace NeuMaterial::App {

// ---------------------------------------------------------------------------
// ValidationIssue
// ---------------------------------------------------------------------------

struct ValidationIssue {
    enum class Severity { Error, Warning };

    std::string field;    // dotted path e.g. "LinearElastic.PoissonsRatio"
    std::string message;
    Severity    severity;
};

// ---------------------------------------------------------------------------
// ValidationResult
// ---------------------------------------------------------------------------

struct ValidationResult {
    std::vector<ValidationIssue> issues;

    /// True if there are no Error-level issues.
    bool isValid() const;

    bool hasErrors()   const;
    bool hasWarnings() const;

    /// All error messages joined as a single string, one per line.
    std::string errorSummary() const;

    /// All warning messages joined as a single string, one per line.
    std::string warningSummary() const;
};

// ---------------------------------------------------------------------------
// MaterialValidator
//
// Validates a Material against its declared Models using the ModelStore for
// property bounds (Minimum/Maximum from the model PropertyDefinition).
//
// Cross-field consistency checks that cannot be expressed in a single
// PropertyDefinition (e.g., YieldStrength <= UltimateStrength) are handled
// here as named rules.
//
// All validation is non-destructive — the Material is never modified.
//
// Unit conversion helpers are pure static functions provided here as a
// convenience for other code that needs to convert between unit systems.
// Naming convention: fromUnit_to_toUnit(value).
// ---------------------------------------------------------------------------

class MaterialValidator {
public:
    MaterialValidator() = delete;

    // ------------------------------------------------------------------
    // Validation
    // ------------------------------------------------------------------

    /// Validate a material against its declared models.
    /// Uses the global ModelStore singleton (must be initialized first).
    static ValidationResult validate(const Material& material);

    // ------------------------------------------------------------------
    // Unit conversion — Density
    // ------------------------------------------------------------------

    static double gcm3_to_kgm3 (double v) { return v * 1e3;       }
    static double kgm3_to_gcm3 (double v) { return v * 1e-3;      }
    static double kgm3_to_lbin3(double v) { return v * 3.6127e-5; }
    static double lbin3_to_kgm3(double v) { return v / 3.6127e-5; }

    // ------------------------------------------------------------------
    // Unit conversion — Pressure / Modulus / Strength
    // ------------------------------------------------------------------

    static double GPa_to_MPa(double v) { return v * 1e3;      }
    static double ksi_to_MPa(double v) { return v * 6.89476;  }
    static double MPa_to_GPa(double v) { return v * 1e-3;     }
    static double MPa_to_psi(double v) { return v * 145.038;  }
    static double psi_to_MPa(double v) { return v / 145.038;  }

    // ------------------------------------------------------------------
    // Unit conversion — Temperature
    // ------------------------------------------------------------------

    static double C_to_F(double v) { return v * 9.0 / 5.0 + 32.0; }
    static double C_to_K(double v) { return v + 273.15;            }
    static double F_to_C(double v) { return (v - 32.0) * 5.0 / 9.0; }
    static double K_to_C(double v) { return v - 273.15;            }

    // ------------------------------------------------------------------
    // Unit conversion — Thermal conductivity
    // ------------------------------------------------------------------

    static double BTU_to_WmK(double v) { return v / 0.57778; }
    static double WmK_to_BTU(double v) { return v * 0.57778; }

private:
    // Per-model-group validators
    static void validateAppearance   (const Material& mat, ValidationResult& r);
    static void validateElectrical   (const Material& mat, ValidationResult& r);
    static void validateLinearElastic(const Material& mat, ValidationResult& r);
    static void validateThermal      (const Material& mat, ValidationResult& r);

    // Model-driven range check using ModelStore PropertyDefinition bounds
    static void checkBounds(const std::string& modelKey,
                            const std::string& propKey,
                            std::optional<double> value,
                            ValidationResult& r);

    // Helpers
    static void addError  (ValidationResult& r, const std::string& field,
                           const std::string& message);
    static void addWarning(ValidationResult& r, const std::string& field,
                           const std::string& message);
};

} // namespace NeuMaterial::App
