// SPDX-FileCopyrightText: 2026 Michael Cummings <mgcummings@yahoo.com>
// SPDX-License-Identifier: CC-BY-4.0

#include "PreCompiled.h"
#include "Material.h"
#include "MaterialValidator.h"
#include "MaterialPy.h"

#include "MaterialPy.cpp"

using namespace NeuMaterial::App;

// ---------------------------------------------------------------------------
// Appearance UUIDs — used to split modelUuids_ into Physical / Appearance
// ---------------------------------------------------------------------------

static const std::vector<std::string> APPEARANCE_UUIDS = {
    "f006c7e4-35b7-43d5-bbf9-c5d572309e6e",  // BasicRendering
};

static bool isAppearanceUuid(const std::string& uuid)
{
    return std::find(APPEARANCE_UUIDS.begin(), APPEARANCE_UUIDS.end(), uuid)
           != APPEARANCE_UUIDS.end();
}

// ---------------------------------------------------------------------------
// Representation
// ---------------------------------------------------------------------------

std::string MaterialPy::representation() const
{
    return "<Material '" + getMaterialPtr()->getName() + "' "
           + getMaterialPtr()->getUuid() + ">";
}

// ---------------------------------------------------------------------------
// Constructor
// ---------------------------------------------------------------------------

PyObject* MaterialPy::PyMake(struct _typeobject*, PyObject*, PyObject*)
{
    return new MaterialPy(new Material());
}

int MaterialPy::PyInit(PyObject* args, PyObject* /*kwd*/)
{
    const char* name = "";
    if (!PyArg_ParseTuple(args, "|s", &name))
        return -1;
    if (*name)
        getMaterialPtr()->setName(name);
    return 0;
}

// ---------------------------------------------------------------------------
// General metadata — getters
// ---------------------------------------------------------------------------

Py::String MaterialPy::getAuthor() const
{
    return Py::String(getMaterialPtr()->getAuthor());
}
void MaterialPy::setAuthor(Py::String v)
{
    getMaterialPtr()->setAuthor(v.as_std_string("utf-8"));
}

Py::String MaterialPy::getDescription() const
{
    return Py::String(getMaterialPtr()->getDescription());
}
void MaterialPy::setDescription(Py::String v)
{
    getMaterialPtr()->setDescription(v.as_std_string("utf-8"));
}

Py::String MaterialPy::getLicense() const
{
    return Py::String(getMaterialPtr()->getLicense());
}
void MaterialPy::setLicense(Py::String v)
{
    getMaterialPtr()->setLicense(v.as_std_string("utf-8"));
}

Py::String MaterialPy::getName() const
{
    return Py::String(getMaterialPtr()->getName());
}
void MaterialPy::setName(Py::String v)
{
    getMaterialPtr()->setName(v.as_std_string("utf-8"));
}

Py::List MaterialPy::getTags() const
{
    Py::List list;
    for (const auto& tag : getMaterialPtr()->getTags())
        list.append(Py::String(tag));
    return list;
}
void MaterialPy::setTags(Py::List v)
{
    std::vector<std::string> tags;
    for (const auto& item : v)
        tags.push_back(Py::String(item).as_std_string("utf-8"));
    getMaterialPtr()->setTags(std::move(tags));
}

Py::String MaterialPy::getURL() const
{
    return Py::String(getMaterialPtr()->getUrl());
}
void MaterialPy::setURL(Py::String v)
{
    getMaterialPtr()->setUrl(v.as_std_string("utf-8"));
}

Py::String MaterialPy::getUUID() const
{
    return Py::String(getMaterialPtr()->getUuid());
}
void MaterialPy::setUUID(Py::String v)
{
    getMaterialPtr()->setUuid(v.as_std_string("utf-8"));
}

Py::String MaterialPy::getVersion() const
{
    return Py::String(getMaterialPtr()->getVersion());
}
void MaterialPy::setVersion(Py::String v)
{
    getMaterialPtr()->setVersion(v.as_std_string("utf-8"));
}

// ---------------------------------------------------------------------------
// Store-level metadata — read-only
// ---------------------------------------------------------------------------

Py::String MaterialPy::getCategory() const
{
    return Py::String(getMaterialPtr()->getCategory());
}

Py::String MaterialPy::getLibrary() const
{
    return Py::String(getMaterialPtr()->getLibrary());
}

Py::Boolean MaterialPy::getReadOnly() const
{
    return Py::Boolean(getMaterialPtr()->isReadOnly());
}

// ---------------------------------------------------------------------------
// Model references
// ---------------------------------------------------------------------------

Py::List MaterialPy::getPhysicalModels() const
{
    Py::List list;
    for (const auto& uuid : getMaterialPtr()->modelUuids())
        if (!isAppearanceUuid(uuid))
            list.append(Py::String(uuid));
    return list;
}

Py::List MaterialPy::getAppearanceModels() const
{
    Py::List list;
    for (const auto& uuid : getMaterialPtr()->modelUuids())
        if (isAppearanceUuid(uuid))
            list.append(Py::String(uuid));
    return list;
}

// ---------------------------------------------------------------------------
// Mechanical properties
// ---------------------------------------------------------------------------

Py::Object MaterialPy::getDensity() const
{
    const auto& v = getMaterialPtr()->mechanical().density;
    if (!v.has_value()) Py_RETURN_NONE;
    return Py::Float(*v);
}
void MaterialPy::setDensity(Py::Object v)
{
    getMaterialPtr()->mechanical().density =
        v.isNone() ? std::optional<double>{} : std::optional<double>{Py::Float(v)};
}

Py::Object MaterialPy::getPoissonsRatio() const
{
    const auto& v = getMaterialPtr()->mechanical().poissonsRatio;
    if (!v.has_value()) Py_RETURN_NONE;
    return Py::Float(*v);
}
void MaterialPy::setPoissonsRatio(Py::Object v)
{
    getMaterialPtr()->mechanical().poissonsRatio =
        v.isNone() ? std::optional<double>{} : std::optional<double>{Py::Float(v)};
}

Py::Object MaterialPy::getUltimateStrength() const
{
    const auto& v = getMaterialPtr()->mechanical().ultimateStrength;
    if (!v.has_value()) Py_RETURN_NONE;
    return Py::Float(*v);
}
void MaterialPy::setUltimateStrength(Py::Object v)
{
    getMaterialPtr()->mechanical().ultimateStrength =
        v.isNone() ? std::optional<double>{} : std::optional<double>{Py::Float(v)};
}

Py::Object MaterialPy::getYieldStrength() const
{
    const auto& v = getMaterialPtr()->mechanical().yieldStrength;
    if (!v.has_value()) Py_RETURN_NONE;
    return Py::Float(*v);
}
void MaterialPy::setYieldStrength(Py::Object v)
{
    getMaterialPtr()->mechanical().yieldStrength =
        v.isNone() ? std::optional<double>{} : std::optional<double>{Py::Float(v)};
}

Py::Object MaterialPy::getYoungsModulus() const
{
    const auto& v = getMaterialPtr()->mechanical().youngsModulus;
    if (!v.has_value()) Py_RETURN_NONE;
    return Py::Float(*v);
}
void MaterialPy::setYoungsModulus(Py::Object v)
{
    getMaterialPtr()->mechanical().youngsModulus =
        v.isNone() ? std::optional<double>{} : std::optional<double>{Py::Float(v)};
}

// ---------------------------------------------------------------------------
// Thermal properties
// ---------------------------------------------------------------------------

Py::Object MaterialPy::getMeltingPoint() const
{
    const auto& v = getMaterialPtr()->thermal().meltingPoint;
    if (!v.has_value()) Py_RETURN_NONE;
    return Py::Float(*v);
}
void MaterialPy::setMeltingPoint(Py::Object v)
{
    getMaterialPtr()->thermal().meltingPoint =
        v.isNone() ? std::optional<double>{} : std::optional<double>{Py::Float(v)};
}

Py::Object MaterialPy::getSpecificHeat() const
{
    const auto& v = getMaterialPtr()->thermal().specificHeat;
    if (!v.has_value()) Py_RETURN_NONE;
    return Py::Float(*v);
}
void MaterialPy::setSpecificHeat(Py::Object v)
{
    getMaterialPtr()->thermal().specificHeat =
        v.isNone() ? std::optional<double>{} : std::optional<double>{Py::Float(v)};
}

Py::Object MaterialPy::getThermalConductivity() const
{
    const auto& v = getMaterialPtr()->thermal().thermalConductivity;
    if (!v.has_value()) Py_RETURN_NONE;
    return Py::Float(*v);
}
void MaterialPy::setThermalConductivity(Py::Object v)
{
    getMaterialPtr()->thermal().thermalConductivity =
        v.isNone() ? std::optional<double>{} : std::optional<double>{Py::Float(v)};
}

Py::Object MaterialPy::getThermalExpansion() const
{
    const auto& v = getMaterialPtr()->thermal().thermalExpansion;
    if (!v.has_value()) Py_RETURN_NONE;
    return Py::Float(*v);
}
void MaterialPy::setThermalExpansion(Py::Object v)
{
    getMaterialPtr()->thermal().thermalExpansion =
        v.isNone() ? std::optional<double>{} : std::optional<double>{Py::Float(v)};
}

// ---------------------------------------------------------------------------
// Electrical properties
// ---------------------------------------------------------------------------

Py::Object MaterialPy::getElectricalConductivity() const
{
    const auto& v = getMaterialPtr()->electrical().electricalConductivity;
    if (!v.has_value()) Py_RETURN_NONE;
    return Py::Float(*v);
}
void MaterialPy::setElectricalConductivity(Py::Object v)
{
    getMaterialPtr()->electrical().electricalConductivity =
        v.isNone() ? std::optional<double>{} : std::optional<double>{Py::Float(v)};
}

Py::Object MaterialPy::getPermittivity() const
{
    const auto& v = getMaterialPtr()->electrical().permittivity;
    if (!v.has_value()) Py_RETURN_NONE;
    return Py::Float(*v);
}
void MaterialPy::setPermittivity(Py::Object v)
{
    getMaterialPtr()->electrical().permittivity =
        v.isNone() ? std::optional<double>{} : std::optional<double>{Py::Float(v)};
}

Py::Object MaterialPy::getResistivity() const
{
    const auto& v = getMaterialPtr()->electrical().resistivity;
    if (!v.has_value()) Py_RETURN_NONE;
    return Py::Float(*v);
}
void MaterialPy::setResistivity(Py::Object v)
{
    getMaterialPtr()->electrical().resistivity =
        v.isNone() ? std::optional<double>{} : std::optional<double>{Py::Float(v)};
}

// ---------------------------------------------------------------------------
// Appearance properties — Phong / BasicRendering model
// ---------------------------------------------------------------------------

namespace {
// Helper to convert std::array<float,4> to a Python list
Py::List colorToList(const std::array<float, 4>& c)
{
    Py::List list;
    for (float f : c)
        list.append(Py::Float(static_cast<double>(f)));
    return list;
}

// Helper to set std::array<float,4> from a Python list [R,G,B] or [R,G,B,A]
void listToColor(Py::List v, std::array<float, 4>& c)
{
    if (v.size() == 3) {
        c = { static_cast<float>(Py::Float(v[0])),
              static_cast<float>(Py::Float(v[1])),
              static_cast<float>(Py::Float(v[2])),
              1.0f };
    } else if (v.size() == 4) {
        c = { static_cast<float>(Py::Float(v[0])),
              static_cast<float>(Py::Float(v[1])),
              static_cast<float>(Py::Float(v[2])),
              static_cast<float>(Py::Float(v[3])) };
    } else {
        throw Py::ValueError("Color must be a list of 3 or 4 floats");
    }
}
} // anonymous namespace

Py::List MaterialPy::getAmbientColor() const
{
    return colorToList(getMaterialPtr()->appearance().ambientColor);
}
void MaterialPy::setAmbientColor(Py::List v)
{
    listToColor(v, getMaterialPtr()->appearance().ambientColor);
}

Py::List MaterialPy::getDiffuseColor() const
{
    return colorToList(getMaterialPtr()->appearance().diffuseColor);
}
void MaterialPy::setDiffuseColor(Py::List v)
{
    listToColor(v, getMaterialPtr()->appearance().diffuseColor);
}

Py::List MaterialPy::getEmissiveColor() const
{
    return colorToList(getMaterialPtr()->appearance().emissiveColor);
}
void MaterialPy::setEmissiveColor(Py::List v)
{
    listToColor(v, getMaterialPtr()->appearance().emissiveColor);
}

Py::Float MaterialPy::getShininess() const
{
    return Py::Float(getMaterialPtr()->appearance().shininess);
}
void MaterialPy::setShininess(Py::Float v)
{
    getMaterialPtr()->appearance().shininess = static_cast<float>(v);
}

Py::List MaterialPy::getSpecularColor() const
{
    return colorToList(getMaterialPtr()->appearance().specularColor);
}
void MaterialPy::setSpecularColor(Py::List v)
{
    listToColor(v, getMaterialPtr()->appearance().specularColor);
}

Py::Float MaterialPy::getTransparency() const
{
    return Py::Float(getMaterialPtr()->appearance().transparency);
}
void MaterialPy::setTransparency(Py::Float v)
{
    getMaterialPtr()->appearance().transparency = static_cast<float>(v);
}

// ---------------------------------------------------------------------------
// Methods
// ---------------------------------------------------------------------------

PyObject* MaterialPy::isValid(PyObject* /*args*/)
{
    return PyBool_FromLong(getMaterialPtr()->isValid() ? 1 : 0);
}

PyObject* MaterialPy::addModelUuid(PyObject* args)
{
    const char* uuid = nullptr;
    if (!PyArg_ParseTuple(args, "s", &uuid))
        return nullptr;
    getMaterialPtr()->addModelUuid(uuid);
    Py_RETURN_NONE;
}

PyObject* MaterialPy::clearModelUuids(PyObject* /*args*/)
{
    getMaterialPtr()->clearModelUuids();
    Py_RETURN_NONE;
}

PyObject* MaterialPy::hasModelUuid(PyObject* args)
{
    const char* uuid = nullptr;
    if (!PyArg_ParseTuple(args, "s", &uuid))
        return nullptr;
    const auto& uuids = getMaterialPtr()->modelUuids();
    bool found = std::find(uuids.begin(), uuids.end(), uuid) != uuids.end();
    return PyBool_FromLong(found ? 1 : 0);
}

PyObject* MaterialPy::validate(PyObject* /*args*/)
{
    // Returns a dict: {'valid': bool, 'errors': [str], 'warnings': [str]}
    auto result = MaterialValidator::validate(*getMaterialPtr());

    PyObject* errors   = PyList_New(0);
    PyObject* warnings = PyList_New(0);

    for (const auto& issue : result.issues) {
        std::string msg = "[" + issue.field + "] " + issue.message;
        PyObject* s = PyUnicode_FromString(msg.c_str());
        if (issue.severity == ValidationIssue::Severity::Error)
            PyList_Append(errors, s);
        else
            PyList_Append(warnings, s);
        Py_DECREF(s);
    }

    PyObject* d = PyDict_New();
    PyDict_SetItemString(d, "valid",    PyBool_FromLong(result.isValid() ? 1 : 0));
    PyDict_SetItemString(d, "errors",   errors);
    PyDict_SetItemString(d, "warnings", warnings);
    Py_DECREF(errors);
    Py_DECREF(warnings);
    return d;
}

PyObject* MaterialPy::getCustomAttributes(const char* /*attr*/) const
{
    return nullptr;
}

int MaterialPy::setCustomAttributes(const char* /*attr*/, PyObject* /*obj*/)
{
    return 0;
}
