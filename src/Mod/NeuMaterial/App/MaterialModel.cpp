// SPDX-FileCopyrightText: 2026 Michael Cummings <mgcummings@yahoo.com>
// SPDX-License-Identifier: CC-BY-4.0

#include "PreCompiled.h"
#include "MaterialModel.h"

namespace NeuMaterial::App {

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

namespace {

// Keys that are part of the Model: header — not property definitions
const std::vector<std::string> MODEL_HEADER_KEYS = {
    "Description", "DOI", "Name", "URL", "UUID",
};

bool isHeaderKey(const std::string& key)
{
    return std::find(MODEL_HEADER_KEYS.begin(), MODEL_HEADER_KEYS.end(), key)
           != MODEL_HEADER_KEYS.end();
}

std::string requireString(const YAML::Node& node, const char* key,
                          const std::string& source)
{
    if (!node[key] || node[key].IsNull())
        throw std::runtime_error("Model missing required field '"
                                 + std::string(key) + "' in " + source);
    return node[key].as<std::string>();
}

std::string optString(const YAML::Node& node, const char* key,
                      const std::string& defaultVal = "")
{
    if (node[key] && !node[key].IsNull())
        return node[key].as<std::string>();
    return defaultVal;
}

std::optional<double> optDouble(const YAML::Node& node, const char* key)
{
    if (!node[key] || node[key].IsNull()) return std::nullopt;
    const std::string s = node[key].as<std::string>();
    if (s.empty()) return std::nullopt;
    try   { return std::stod(s); }
    catch (...) { return std::nullopt; }
}

} // anonymous namespace

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------

const PropertyDefinition* MaterialModel::property(const std::string& name) const
{
    auto it = properties_.find(name);
    return it != properties_.end() ? &it->second : nullptr;
}

bool MaterialModel::hasProperty(const std::string& name) const
{
    return properties_.count(name) > 0;
}

MaterialModel MaterialModel::loadFromFile(const std::filesystem::path& path)
{
    try {
        YAML::Node root = YAML::LoadFile(path.string());
        return parseNode(root, path.string());
    }
    catch (const YAML::Exception& e) {
        throw std::runtime_error("YAML parse error in model file "
                                 + path.string() + ": " + e.what());
    }
}

MaterialModel MaterialModel::loadFromString(const std::string& yaml)
{
    try {
        YAML::Node root = YAML::Load(yaml);
        return parseNode(root, "<string>");
    }
    catch (const YAML::Exception& e) {
        throw std::runtime_error(std::string("YAML parse error: ") + e.what());
    }
}

// ---------------------------------------------------------------------------
// Private — parsing
// ---------------------------------------------------------------------------

MaterialModel MaterialModel::parseNode(const YAML::Node& root,
                                       const std::string& source)
{
    // Determine whether this is a physical model ('Model:') or an
    // appearance model ('AppearanceModel:') and set the node accordingly
    YAML::Node m;
    bool isAppearance = false;

    if (root["AppearanceModel"] && !root["AppearanceModel"].IsNull()) {
        m             = root["AppearanceModel"];
        isAppearance  = true;
    }
    else if (root["Model"] && !root["Model"].IsNull()) {
        m = root["Model"];
    }
    else {
        throw std::runtime_error(
            "Model file missing 'Model:' or 'AppearanceModel:' key in "
            + source);
    }

    MaterialModel model;
    model.appearanceModel_ = isAppearance;

    // ------------------------------------------------------------------
    // Header — UpperCamelCase keys (same structure for both types)
    // ------------------------------------------------------------------
    model.name_        = requireString(m, "Name",        source);
    model.uuid_        = requireString(m, "UUID",        source);
    model.description_ = requireString(m, "Description", source);
    model.url_         = optString    (m, "URL");
    model.doi_         = optString    (m, "DOI");

    // ------------------------------------------------------------------
    // Inherited: block — composite models only (top-level, not under Model:)
    // Uses UpperCamelCase UUID key
    // ------------------------------------------------------------------
    if (root["Inherited"] && !root["Inherited"].IsNull()) {
        for (const auto& entry : root["Inherited"]) {
            InheritedModel inh;
            inh.name = entry.first.as<std::string>();
            const YAML::Node& val = entry.second;
            if (!val["UUID"] || val["UUID"].IsNull())
                throw std::runtime_error("Inherited entry '" + inh.name
                                         + "' missing UUID in " + source);
            inh.uuid = val["UUID"].as<std::string>();
            model.inherited_.push_back(std::move(inh));
        }
    }

    // ------------------------------------------------------------------
    // Property definitions — sit directly under Model: alongside the
    // header keys. Any key that is not a known header key is a property.
    //
    // Expected structure per property:
    //   PropertyName:
    //     DisplayName: 'Human Readable Name'
    //     Type: 'Quantity'
    //     Units: 'kg/m^3'
    //     URL: '...'
    //     Description: '...'
    //     Minimum: '0.0'    # optional
    //     Maximum: ''       # optional, empty = no bound
    // ------------------------------------------------------------------
    for (const auto& entry : m) {
        const std::string key = entry.first.as<std::string>();

        // Skip header keys
        if (isHeaderKey(key)) continue;

        const YAML::Node& pn = entry.second;

        // Must be a map to be a valid property definition
        if (!pn.IsMap()) continue;

        PropertyDefinition def;
        def.description = optString(pn, "Description");
        def.displayName = optString(pn, "DisplayName", key);
        def.maximum     = optDouble(pn, "Maximum");
        def.minimum     = optDouble(pn, "Minimum");
        def.name        = key;
        def.type        = optString(pn, "Type",  "Quantity");
        def.units       = optString(pn, "Units", "");
        def.url         = optString(pn, "URL",   "");

        model.properties_.emplace(key, std::move(def));
    }

    return model;
}

} // namespace NeuMaterial::App
