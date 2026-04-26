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
    "3df55a7e-7c4d-4b6b-8b3a-5a5e6e7f8a9b",  // RenderAppearance
    "f2c3d5f7-3d4e-4a6f-7b8c-9d0e1f2a3b4c",  // Color
    "b4e5f7b9-5f6a-4c8b-9d0e-1f2a3b4c5d6e",  // Metallic
    "c5f6a8ca-6a7b-4d9c-0e1f-2a3b4c5d6e7f",  // Opacity
    "a3d4e6a8-4e5f-4b7a-8c9d-0e1f2a3b4c5d",  // Roughness
    "d6a7b9db-7b8c-4e0d-1f2a-3b4c5d6e7f8a",  // TexturePath
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
// Appearance properties
// ---------------------------------------------------------------------------

Py::List MaterialPy::getColor() const
{
    const auto& c = getMaterialPtr()->appearance().color;
    Py::List list;
    for (float f : c)
        list.append(Py::Float(static_cast<double>(f)));
    return list;
}
void MaterialPy::setColor(Py::List v)
{
    auto& c = getMaterialPtr()->appearance().color;
    if (v.size() == 3) {
        c[0] = static_cast<float>(Py::Float(v[0]));
        c[1] = static_cast<float>(Py::Float(v[1]));
        c[2] = static_cast<float>(Py::Float(v[2]));
        c[3] = 1.0f;
    } else if (v.size() == 4) {
        c[0] = static_cast<float>(Py::Float(v[0]));
        c[1] = static_cast<float>(Py::Float(v[1]));
        c[2] = static_cast<float>(Py::Float(v[2]));
        c[3] = static_cast<float>(Py::Float(v[3]));
    } else {
        throw Py::ValueError("Color must be a list of 3 or 4 floats");
    }
}

Py::Float MaterialPy::getMetallic() const
{
    return Py::Float(getMaterialPtr()->appearance().metallic);
}
void MaterialPy::setMetallic(Py::Float v)
{
    getMaterialPtr()->appearance().metallic = static_cast<float>(v);
}

Py::Float MaterialPy::getOpacity() const
{
    return Py::Float(getMaterialPtr()->appearance().opacity);
}
void MaterialPy::setOpacity(Py::Float v)
{
    getMaterialPtr()->appearance().opacity = static_cast<float>(v);
}

Py::Float MaterialPy::getRoughness() const
{
    return Py::Float(getMaterialPtr()->appearance().roughness);
}
void MaterialPy::setRoughness(Py::Float v)
{
    getMaterialPtr()->appearance().roughness = static_cast<float>(v);
}

Py::String MaterialPy::getTexturePath() const
{
    return Py::String(getMaterialPtr()->appearance().texturePath);
}
void MaterialPy::setTexturePath(Py::String v)
{
    getMaterialPtr()->appearance().texturePath = v.as_std_string("utf-8");
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
