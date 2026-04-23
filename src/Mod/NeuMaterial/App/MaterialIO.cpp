#include "PreCompiled.h"
#include "MaterialIO.h"

#include <fstream>
#include <stdexcept>

namespace NeuMaterial::App {

// ---------------------------------------------------------------------------
// Helpers — optional<double> read/write
// ---------------------------------------------------------------------------

namespace {

template<typename T>
std::optional<T> optionalFrom(const YAML::Node& node, const char* key)
{
    if (node[key] && !node[key].IsNull())
        return node[key].as<T>();
    return std::nullopt;
}

void emitOptional(YAML::Emitter& out, const char* key,
                  const std::optional<double>& value)
{
    out << YAML::Key << key;
    if (value.has_value())
        out << YAML::Value << *value;
    else
        out << YAML::Value << YAML::Null;
}

} // anonymous namespace

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------

Material MaterialIO::load(const std::filesystem::path& filePath)
{
    try {
        YAML::Node root = YAML::LoadFile(filePath.string());
        return parseNode(root);
    } catch (const YAML::Exception& e) {
        throw std::runtime_error("YAML parse error in " + filePath.string()
                                 + ": " + e.what());
    }
}

Material MaterialIO::loadFromString(const std::string& yaml)
{
    try {
        YAML::Node root = YAML::Load(yaml);
        return parseNode(root);
    } catch (const YAML::Exception& e) {
        throw std::runtime_error(std::string("YAML parse error: ") + e.what());
    }
}

void MaterialIO::save(const Material& material,
                      const std::filesystem::path& filePath)
{
    std::ofstream ofs(filePath);
    if (!ofs)
        throw std::runtime_error("Cannot open file for writing: " + filePath.string());

    ofs << toString(material);
}

std::string MaterialIO::toString(const Material& material)
{
    YAML::Emitter out;
    out << toNode(material);
    return std::string(out.c_str());
}

// ---------------------------------------------------------------------------
// Private — parsing
// ---------------------------------------------------------------------------

Material MaterialIO::parseNode(const YAML::Node& root)
{
    if (!root["name"])
        throw std::runtime_error("Material YAML missing required field 'name'");

    Material mat(root["name"].as<std::string>());

    // Restore stored id if present (preserves identity across sessions)
    if (root["id"] && !root["id"].IsNull())
        const_cast<std::string&>(mat.getId()) = root["id"].as<std::string>();
    // Note: getId() returns const ref; we'd normally use a friend or setter.
    // A cleaner alternative is a setId() method — add to Material.h as needed.

    if (root["description"])
        mat.setDescription(root["description"].as<std::string>());
    if (root["category"])
        mat.setCategory(root["category"].as<std::string>());

    // Mechanical
    if (const YAML::Node& m = root["mechanical"]) {
        auto& mp          = mat.mechanical();
        mp.density         = optionalFrom<double>(m, "density");
        mp.youngsModulus   = optionalFrom<double>(m, "youngs_modulus");
        mp.poissonsRatio   = optionalFrom<double>(m, "poissons_ratio");
        mp.yieldStrength   = optionalFrom<double>(m, "yield_strength");
        mp.ultimateStrength = optionalFrom<double>(m, "ultimate_strength");
    }

    // Thermal
    if (const YAML::Node& t = root["thermal"]) {
        auto& tp               = mat.thermal();
        tp.thermalConductivity  = optionalFrom<double>(t, "thermal_conductivity");
        tp.thermalExpansion     = optionalFrom<double>(t, "thermal_expansion");
        tp.specificHeat         = optionalFrom<double>(t, "specific_heat");
        tp.meltingPoint         = optionalFrom<double>(t, "melting_point");
    }

    // Electrical
    if (const YAML::Node& e = root["electrical"]) {
        auto& ep                   = mat.electrical();
        ep.electricalConductivity   = optionalFrom<double>(e, "electrical_conductivity");
        ep.permittivity             = optionalFrom<double>(e, "permittivity");
        ep.resistivity              = optionalFrom<double>(e, "resistivity");
    }

    // Appearance
    if (const YAML::Node& a = root["appearance"]) {
        auto& ap = mat.appearance();

        if (a["color"] && a["color"].IsSequence() && a["color"].size() == 4) {
            ap.color[0] = a["color"][0].as<float>();
            ap.color[1] = a["color"][1].as<float>();
            ap.color[2] = a["color"][2].as<float>();
            ap.color[3] = a["color"][3].as<float>();
        }
        if (a["texture_path"] && !a["texture_path"].IsNull())
            ap.texturePath = a["texture_path"].as<std::string>();
        if (a["roughness"] && !a["roughness"].IsNull())
            ap.roughness = a["roughness"].as<float>();
        if (a["metallic"] && !a["metallic"].IsNull())
            ap.metallic = a["metallic"].as<float>();
        if (a["opacity"] && !a["opacity"].IsNull())
            ap.opacity = a["opacity"].as<float>();
    }

    return mat;
}

// ---------------------------------------------------------------------------
// Private — serialisation
// ---------------------------------------------------------------------------

YAML::Node MaterialIO::toNode(const Material& material)
{
    YAML::Emitter out;
    out << YAML::BeginMap;

    // Identity
    out << YAML::Key << "id"          << YAML::Value << material.getId();
    out << YAML::Key << "name"        << YAML::Value << material.getName();
    out << YAML::Key << "description" << YAML::Value << material.getDescription();
    out << YAML::Key << "category"    << YAML::Value << material.getCategory();

    // Mechanical
    out << YAML::Key << "mechanical" << YAML::Value << YAML::BeginMap;
    emitOptional(out, "density",           material.mechanical().density);
    emitOptional(out, "youngs_modulus",    material.mechanical().youngsModulus);
    emitOptional(out, "poissons_ratio",    material.mechanical().poissonsRatio);
    emitOptional(out, "yield_strength",    material.mechanical().yieldStrength);
    emitOptional(out, "ultimate_strength", material.mechanical().ultimateStrength);
    out << YAML::EndMap;

    // Thermal
    out << YAML::Key << "thermal" << YAML::Value << YAML::BeginMap;
    emitOptional(out, "thermal_conductivity", material.thermal().thermalConductivity);
    emitOptional(out, "thermal_expansion",    material.thermal().thermalExpansion);
    emitOptional(out, "specific_heat",        material.thermal().specificHeat);
    emitOptional(out, "melting_point",        material.thermal().meltingPoint);
    out << YAML::EndMap;

    // Electrical
    out << YAML::Key << "electrical" << YAML::Value << YAML::BeginMap;
    emitOptional(out, "electrical_conductivity", material.electrical().electricalConductivity);
    emitOptional(out, "permittivity",            material.electrical().permittivity);
    emitOptional(out, "resistivity",             material.electrical().resistivity);
    out << YAML::EndMap;

    // Appearance
    const auto& ap = material.appearance();
    out << YAML::Key << "appearance" << YAML::Value << YAML::BeginMap;
    out << YAML::Key << "color" << YAML::Value
        << YAML::Flow << YAML::BeginSeq
        << ap.color[0] << ap.color[1] << ap.color[2] << ap.color[3]
        << YAML::EndSeq;
    out << YAML::Key << "texture_path"
        << YAML::Value << (ap.texturePath.empty() ? YAML::Null : YAML::Node(ap.texturePath));
    out << YAML::Key << "roughness" << YAML::Value << ap.roughness;
    out << YAML::Key << "metallic"  << YAML::Value << ap.metallic;
    out << YAML::Key << "opacity"   << YAML::Value << ap.opacity;
    out << YAML::EndMap;

    out << YAML::EndMap;

    // Return as a parsed Node so callers can compose it further if needed
    return YAML::Load(out.c_str());
}

} // namespace NeuMaterial::App
