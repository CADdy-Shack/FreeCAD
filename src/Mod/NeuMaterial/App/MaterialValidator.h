#pragma once

#include "Material.h"

#include <string>
#include <vector>

namespace NeuMaterial::App {

// ---------------------------------------------------------------------------
// ValidationResult
// ---------------------------------------------------------------------------

struct ValidationIssue {
    enum class Severity { Warning, Error };

    Severity    severity;
    std::string field;    // dotted path, e.g. "mechanical.poissons_ratio"
    std::string message;
};

struct ValidationResult {
    std::vector<ValidationIssue> issues;

    bool isValid() const;    // true if no Error-level issues
    bool hasWarnings() const;

    /// Convenience: collect all error messages as a single string
    std::string errorSummary() const;
};

// ---------------------------------------------------------------------------
// MaterialValidator
//
// Validates property ranges and optionally converts between unit systems.
// All validation is non-destructive — the original Material is never modified.
// ---------------------------------------------------------------------------

class MaterialValidator {
public:
    MaterialValidator() = delete;

    // ------------------------------------------------------------------
    // Validation
    // ------------------------------------------------------------------

    static ValidationResult validate(const Material& material);

    // ------------------------------------------------------------------
    // Unit conversion helpers
    //
    // These are pure functions: given a value in the source unit,
    // return the equivalent in the target unit.
    //
    // Naming convention: <quantity>_<fromUnit>_to_<toUnit>
    // ------------------------------------------------------------------

    // Pressure / modulus  (MPa ↔ GPa ↔ psi ↔ ksi)
    static double MPa_to_GPa(double mpa)  { return mpa  * 1e-3; }
    static double GPa_to_MPa(double gpa)  { return gpa  * 1e3;  }
    static double MPa_to_psi(double mpa)  { return mpa  * 145.038; }
    static double psi_to_MPa(double psi)  { return psi  / 145.038; }
    static double ksi_to_MPa(double ksi)  { return ksi  * 6.89476; }

    // Density  (kg/m³ ↔ g/cm³ ↔ lb/in³)
    static double kgm3_to_gcm3(double v)  { return v * 1e-3;      }
    static double gcm3_to_kgm3(double v)  { return v * 1e3;       }
    static double kgm3_to_lbin3(double v) { return v * 3.6127e-5; }
    static double lbin3_to_kgm3(double v) { return v / 3.6127e-5; }

    // Temperature  (°C ↔ °F ↔ K)
    static double C_to_F(double c)        { return c * 9.0 / 5.0 + 32.0; }
    static double F_to_C(double f)        { return (f - 32.0) * 5.0 / 9.0; }
    static double C_to_K(double c)        { return c + 273.15; }
    static double K_to_C(double k)        { return k - 273.15; }

    // Thermal conductivity  (W/m·K ↔ BTU/h·ft·°F)
    static double WmK_to_BTU(double v)    { return v * 0.57778; }
    static double BTU_to_WmK(double v)    { return v / 0.57778; }

private:
    static void validateMechanical(const Material& mat, ValidationResult& result);
    static void validateThermal   (const Material& mat, ValidationResult& result);
    static void validateElectrical(const Material& mat, ValidationResult& result);
    static void validateAppearance(const Material& mat, ValidationResult& result);

    static void addError  (ValidationResult& r, const std::string& field,
                           const std::string& message);
    static void addWarning(ValidationResult& r, const std::string& field,
                           const std::string& message);
};

} // namespace NeuMaterial::App
