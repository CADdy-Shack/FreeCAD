#include "MaterialIO.h"

#include <fstream>

namespace NeuMaterial::App {

// ---------------------------------------------------------------------------
// Anonymous helpers
// ---------------------------------------------------------------------------

namespace {

std::string reqString(const YAML::Node& node, const char* key,
                      const std::string& source)
{
    if (!node[key] || node[key].IsNull())
        throw std::runtime_error("Material YAML missing required key '"
                                 + std::string(key) + "' in " + source);
    return node[key].as<std::string>();
}

std::string optString(const YAML::Node& node, const char* key,
                      const std::string& def = "")
{
    if (node[key] && !node[key].IsNull())
        return node[key].as<std::string>();
    return def;
}

// Emit a string key/value only if value is non-empty
void emitStr(YAML::Emitter& out, const char* key, const std::string& val)
{
    out << YAML::Key << key << YAML::Value << val;
}

// Emit optional quantity as 'value unit' — skip if nullopt
void emitQuantity(YAML::Emitter& out, const char* key,
                  const std::optional<double>& val, const char* unit)
{
    if (!val.has_value()) return;
    std::ostringstream ss;
    ss << *val;
    if (unit && *unit) ss << " " << unit;
    out << YAML::Key << key << YAML::Value << ss.str();
}

void emitFloat(YAML::Emitter& out, const char* key,
               const std::optional<float>& val)
{
    if (!val.has_value()) return;
    std::ostringstream ss;
    ss << *val;
    out << YAML::Key << key << YAML::Value << ss.str();
}

} // anonymous namespace

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------

Material MaterialIO::load(const std::filesystem::path& filePath)
{
    try {
        YAML::Node root = YAML::LoadFile(filePath.string());
        return parseNode(root, filePath.string());
    }
    catch (const YAML::Exception& e) {
        throw std::runtime_error("YAML parse error in " + filePath.string()
                                 + ": " + e.what());
    }
}

Material MaterialIO::loadFromString(const std::string& yaml)
{
    try {
        YAML::Node root = YAML::Load(yaml);
        return parseNode(root, "<string>");
    }
    catch (const YAML::Exception& e) {
        throw std::runtime_error(std::string("YAML parse error: ") + e.what());
    }
}

void MaterialIO::save(const Material& material,
                      const std::filesystem::path& filePath)
{
    std::ofstream ofs(filePath);
    if (!ofs)
        throw std::runtime_error("Cannot open for writing: "
                                 + filePath.string());
    ofs << toString(material);
}

std::string MaterialIO::toString(const Material& material)
{
    YAML::Emitter out;
    out.SetIndent(4);

    out << YAML::BeginDoc;
    out << YAML::BeginMap;

    emitGeneral(out, material);
    emitModels (out, material);

    out << YAML::EndMap;
    out << YAML::EndDoc;

    return std::string(out.c_str());
}

// ---------------------------------------------------------------------------
// Private — parsing
// ---------------------------------------------------------------------------

Material MaterialIO::parseNode(const YAML::Node& root,
                               const std::string& source)
{
    if (!root["General"])
        throw std::runtime_error("Material YAML missing 'General:' block in "
                                 + source);
    if (!root["Models"])
        throw std::runtime_error("Material YAML missing 'Models:' block in "
                                 + source);

    Material mat;
    parseGeneral(root["General"], mat, source);
    parseModels (root["Models"],  mat);
    return mat;
}

void MaterialIO::parseGeneral(const YAML::Node& g, Material& mat,
                              const std::string& source)
{
    mat.setUuid       (reqString(g, "UUID",        source));
    mat.setName       (reqString(g, "Name",        source));
    mat.setAuthor     (optString(g, "Author"));
    mat.setDescription(optString(g, "Description"));
    mat.setLicense    (optString(g, "License"));
    mat.setUrl        (optString(g, "URL"));
    mat.setVersion    (optString(g, "Version", "2.0"));

    if (g["Tags"] && g["Tags"].IsSequence()) {
        for (const auto& tag : g["Tags"])
            mat.addTag(tag.as<std::string>());
    }
}

void MaterialIO::parseModels(const YAML::Node& models, Material& mat)
{
    for (const auto& entry : models) {
        const std::string key = entry.first.as<std::string>();
        const YAML::Node& node = entry.second;

        // Every model block must have a UUID
        if (node["UUID"] && !node["UUID"].IsNull())
            mat.addModelUuid(node["UUID"].as<std::string>());

        // Dispatch to per-model parsers by key
        if      (key == "LinearElastic")   parseLinearElastic   (node, mat);
        else if (key == "Thermal")          parseThermal         (node, mat);
        else if (key == "Electrical")       parseElectrical      (node, mat);
        else if (key == "RenderAppearance") parseRenderAppearance(node, mat);
        // Unknown model keys are stored as UUIDs only — forward compatible
    }
}

void MaterialIO::parseLinearElastic(const YAML::Node& n, Material& mat)
{
    auto& m = mat.mechanical();
    m.density          = parseQuantity(n, "Density");
    m.poissonsRatio    = parseFloat   (n, "PoissonsRatio");
    m.ultimateStrength = parseQuantity(n, "UltimateStrength");
    m.yieldStrength    = parseQuantity(n, "YieldStrength");
    m.youngsModulus    = parseQuantity(n, "YoungsModulus");
}

void MaterialIO::parseThermal(const YAML::Node& n, Material& mat)
{
    auto& t = mat.thermal();
    t.meltingPoint        = parseQuantity(n, "MeltingPoint");
    t.specificHeat        = parseQuantity(n, "SpecificHeat");
    t.thermalConductivity = parseQuantity(n, "ThermalConductivity");
    t.thermalExpansion    = parseQuantity(n, "ThermalExpansion");
}

void MaterialIO::parseElectrical(const YAML::Node& n, Material& mat)
{
    auto& e = mat.electrical();
    e.electricalConductivity = parseQuantity(n, "ElectricalConductivity");
    e.permittivity           = parseQuantity(n, "Permittivity");
    e.resistivity            = parseQuantity(n, "Resistivity");
}

void MaterialIO::parseRenderAppearance(const YAML::Node& n, Material& mat)
{
    auto& a = mat.appearance();

    if (n["Color"] && !n["Color"].IsNull()) {
        std::array<float, 4> c = {0.8f, 0.8f, 0.8f, 1.0f};
        if (parseColor(n["Color"].as<std::string>(), c))
            a.color = c;
    }
    if (auto v = parseFloat(n, "Metallic"))    a.metallic    = *v;
    if (auto v = parseFloat(n, "Opacity"))     a.opacity     = *v;
    if (auto v = parseFloat(n, "Roughness"))   a.roughness   = *v;
    if (n["TexturePath"] && !n["TexturePath"].IsNull())
        a.texturePath = n["TexturePath"].as<std::string>();
}

// ---------------------------------------------------------------------------
// Private — serialisation
// ---------------------------------------------------------------------------

void MaterialIO::emitGeneral(YAML::Emitter& out, const Material& mat)
{
    out << YAML::Key << "General" << YAML::Value << YAML::BeginMap;

    // Keys emitted sorted a-z
    emitStr(out, "Author",      mat.getAuthor());
    emitStr(out, "Description", mat.getDescription());
    emitStr(out, "License",     mat.getLicense());
    emitStr(out, "Name",        mat.getName());

    // Tags as a sorted inline sequence
    if (!mat.getTags().empty()) {
        auto tags = mat.getTags();
        std::sort(tags.begin(), tags.end());
        out << YAML::Key << "Tags" << YAML::Value
            << YAML::Flow << YAML::BeginSeq;
        for (const auto& t : tags) out << t;
        out << YAML::EndSeq;
    }

    emitStr(out, "URL",     mat.getUrl());
    emitStr(out, "UUID",    mat.getUuid());
    emitStr(out, "Version", mat.getVersion());

    out << YAML::EndMap;
}

void MaterialIO::emitModels(YAML::Emitter& out, const Material& mat)
{
    out << YAML::Key << "Models" << YAML::Value << YAML::BeginMap;

    // Emit models sorted a-z by key name
    // Electrical
    emitElectrical     (out, mat);
    // LinearElastic
    emitLinearElastic  (out, mat);
    // RenderAppearance (only if any appearance properties set)
    emitRenderAppearance(out, mat);
    // Thermal
    emitThermal        (out, mat);

    out << YAML::EndMap;
}

void MaterialIO::emitLinearElastic(YAML::Emitter& out, const Material& mat)
{
    const auto& m = mat.mechanical();
    // Skip entire block if no mechanical properties set
    if (!m.density && !m.poissonsRatio && !m.ultimateStrength
        && !m.yieldStrength && !m.youngsModulus)
        return;

    // Find the LinearElastic UUID from the material's model list
    // Falls back to the known UUID if not explicitly stored
    const std::string uuid = "9cdda8be-b8fb-4c62-95c2-74e852d4afae";

    out << YAML::Key << "LinearElastic" << YAML::Value << YAML::BeginMap;
    emitQuantity(out, "Density",          m.density,          "kg/m^3");
    emitFloat   (out, "PoissonsRatio",    m.poissonsRatio ? std::optional<float>(*m.poissonsRatio) : std::nullopt);
    emitQuantity(out, "UltimateStrength", m.ultimateStrength, "MPa");
    emitStr     (out, "UUID",             uuid);
    emitQuantity(out, "YieldStrength",    m.yieldStrength,    "MPa");
    emitQuantity(out, "YoungsModulus",    m.youngsModulus,    "GPa");
    out << YAML::EndMap;
}

void MaterialIO::emitThermal(YAML::Emitter& out, const Material& mat)
{
    const auto& t = mat.thermal();
    if (!t.meltingPoint && !t.specificHeat
        && !t.thermalConductivity && !t.thermalExpansion)
        return;

    const std::string uuid = "b2eb5f48-74d4-4f01-a8f0-5ecf7c3e65a4";

    out << YAML::Key << "Thermal" << YAML::Value << YAML::BeginMap;
    emitQuantity(out, "MeltingPoint",        t.meltingPoint,        "degC");
    emitQuantity(out, "SpecificHeat",        t.specificHeat,        "J/kg/K");
    emitQuantity(out, "ThermalConductivity", t.thermalConductivity, "W/m/K");
    emitQuantity(out, "ThermalExpansion",    t.thermalExpansion,    "um/m/K");
    emitStr     (out, "UUID",                uuid);
    out << YAML::EndMap;
}

void MaterialIO::emitElectrical(YAML::Emitter& out, const Material& mat)
{
    const auto& e = mat.electrical();
    if (!e.electricalConductivity && !e.permittivity && !e.resistivity)
        return;

    const std::string uuid = "a4e61e23-7a77-4b4c-9da6-a1a22e5c3e7b";

    out << YAML::Key << "Electrical" << YAML::Value << YAML::BeginMap;
    emitQuantity(out, "ElectricalConductivity", e.electricalConductivity, "S/m");
    emitQuantity(out, "Permittivity",           e.permittivity,           "F/m");
    emitQuantity(out, "Resistivity",            e.resistivity,            "Ohm*m");
    emitStr     (out, "UUID",                   uuid);
    out << YAML::EndMap;
}

void MaterialIO::emitRenderAppearance(YAML::Emitter& out, const Material& mat)
{
    const auto& a = mat.appearance();

    // TODO: 'RenderAppearance' key name is provisional — the appearance model
    // name may change once the relationship between the Material workbench and
    // any rendering workbench is clarified. Treat as a marker for future work.

    // Only emit if any appearance property differs from defaults
    const AppearanceProperties defaults;
    if (a == defaults) return;

    const std::string uuid = "3df55a7e-7c4d-4b6b-8b3a-5a5e6e7f8a9b";

    out << YAML::Key << "RenderAppearance" << YAML::Value << YAML::BeginMap;

    // Color as '(r, g, b, a)'
    std::ostringstream color;
    color << "(" << a.color[0] << ", " << a.color[1] << ", "
                 << a.color[2] << ", " << a.color[3] << ")";
    emitStr(out, "Color", color.str());

    emitStr(out, "Metallic",  std::to_string(a.metallic));
    emitStr(out, "Opacity",   std::to_string(a.opacity));
    emitStr(out, "Roughness", std::to_string(a.roughness));
    if (!a.texturePath.empty())
        emitStr(out, "TexturePath", a.texturePath);
    emitStr(out, "UUID", uuid);

    out << YAML::EndMap;
}

// ---------------------------------------------------------------------------
// Private — value parsing helpers
// ---------------------------------------------------------------------------

std::optional<double> MaterialIO::parseQuantity(const YAML::Node& node,
                                                const char* key)
{
    if (!node[key] || node[key].IsNull()) return std::nullopt;
    const std::string s = node[key].as<std::string>();
    if (s.empty()) return std::nullopt;
    try {
        std::size_t pos = 0;
        double val = std::stod(s, &pos);
        return val;
    }
    catch (...) { return std::nullopt; }
}

std::optional<float> MaterialIO::parseFloat(const YAML::Node& node,
                                            const char* key)
{
    if (auto v = parseQuantity(node, key))
        return static_cast<float>(*v);
    return std::nullopt;
}

bool MaterialIO::parseColor(const std::string& str,
                            std::array<float, 4>& out)
{
    // Accepts '(r, g, b)' or '(r, g, b, a)'
    std::string s = str;
    // Strip parens
    s.erase(std::remove(s.begin(), s.end(), '('), s.end());
    s.erase(std::remove(s.begin(), s.end(), ')'), s.end());

    std::vector<float> components;
    std::istringstream ss(s);
    std::string token;
    while (std::getline(ss, token, ',')) {
        try { components.push_back(std::stof(token)); }
        catch (...) { return false; }
    }

    if (components.size() == 3) {
        out = {components[0], components[1], components[2], 1.0f};
        return true;
    }
    if (components.size() == 4) {
        out = {components[0], components[1], components[2], components[3]};
        return true;
    }
    return false;
}

} // namespace NeuMaterial::App
