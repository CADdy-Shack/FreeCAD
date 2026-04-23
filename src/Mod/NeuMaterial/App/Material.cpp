#include "PreCompiled.h"
#include "Material.h"

#include <algorithm>
#include <cctype>

namespace NeuMaterial::App {

// ---------------------------------------------------------------------------
// Construction
// ---------------------------------------------------------------------------

Material::Material(std::string name)
    : name_(std::move(name))
{
    // UUID is intentionally left empty on construction — it must be set
    // explicitly via setUuid() when loading from YAML or assigned by the
    // user before calling MaterialStore::addMaterial().
}

// ---------------------------------------------------------------------------
// Validation
// ---------------------------------------------------------------------------

bool Material::isValid() const
{
    return !name_.empty() && !uuid_.empty();
}

// ---------------------------------------------------------------------------
// Static helpers
// ---------------------------------------------------------------------------

std::string Material::makeSlug(const std::string& name)
{
    std::string slug;
    slug.reserve(name.size());

    for (const char c : name) {
        if (std::isalnum(static_cast<unsigned char>(c))) {
            slug += static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        }
        else if (std::isspace(static_cast<unsigned char>(c)) ||
                 c == '-' || c == '_') {
            // Collapse runs of separators to a single underscore
            if (!slug.empty() && slug.back() != '_')
                slug += '_';
                 }
        // All other characters stripped
    }

    // Trim trailing underscore
    while (!slug.empty() && slug.back() == '_')
        slug.pop_back();

    return slug;
}

} // namespace NeuMaterial::App
