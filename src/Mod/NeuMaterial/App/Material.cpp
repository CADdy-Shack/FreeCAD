#include "PreCompiled.h"
#include "Material.h"

#include <algorithm>
#include <cctype>
#include <random>
#include <sstream>
#include <iomanip>

namespace NeuMaterial::App {

// ---------------------------------------------------------------------------
// Construction
// ---------------------------------------------------------------------------

Material::Material(std::string name)
    : name_(std::move(name))
{
    generateId();
}

// ---------------------------------------------------------------------------
// Identity setters
// ---------------------------------------------------------------------------

void Material::setName(std::string name)
{
    name_ = std::move(name);
    // Re-generate id only if it hasn't been set from a stored file
    if (id_.empty())
        generateId();
}

// ---------------------------------------------------------------------------
// Validation
// ---------------------------------------------------------------------------

bool Material::isValid() const
{
    return !name_.empty();
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
        } else if (std::isspace(static_cast<unsigned char>(c)) ||
                   c == '-' || c == '_') {
            // Collapse runs of separators to a single underscore
            if (!slug.empty() && slug.back() != '_')
                slug += '_';
        }
        // Strip all other characters
    }

    // Trim trailing underscore
    while (!slug.empty() && slug.back() == '_')
        slug.pop_back();

    return slug;
}

// ---------------------------------------------------------------------------
// Private helpers
// ---------------------------------------------------------------------------

void Material::generateId()
{
    // Simple UUID-v4-like generator — no external dependency required
    std::random_device              rd;
    std::mt19937                    gen(rd());
    std::uniform_int_distribution<> dis(0, 15);
    std::uniform_int_distribution<> dis2(8, 11);

    std::ostringstream oss;
    oss << std::hex;

    for (int i = 0; i < 8;  ++i) oss << dis(gen);
    oss << '-';
    for (int i = 0; i < 4;  ++i) oss << dis(gen);
    oss << "-4";
    for (int i = 0; i < 3;  ++i) oss << dis(gen);
    oss << '-';
    oss << dis2(gen);
    for (int i = 0; i < 3;  ++i) oss << dis(gen);
    oss << '-';
    for (int i = 0; i < 12; ++i) oss << dis(gen);

    id_ = oss.str();
}

} // namespace NeuMaterial::App
