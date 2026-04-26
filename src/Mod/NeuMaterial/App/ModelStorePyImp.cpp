// SPDX-FileCopyrightText: 2026 Michael Cummings <mgcummings@yahoo.com>
// SPDX-License-Identifier: CC-BY-4.0

#include "PreCompiled.h"
#include "MaterialModel.h"
#include "ModelStore.h"
#include "ModelStorePy.h"
#include "NeuMaterialApp.h"

#include "ModelStorePy.cpp"

using namespace NeuMaterial::App;

// ---------------------------------------------------------------------------
// Helper — convert a PropertyDefinition to a Python dict
// ---------------------------------------------------------------------------

static PyObject* propDefToDict(const PropertyDefinition& def)
{
    PyObject* d = PyDict_New();
    PyDict_SetItemString(d, "name",
        PyUnicode_FromString(def.name.c_str()));
    PyDict_SetItemString(d, "type",
        PyUnicode_FromString(def.type.c_str()));
    PyDict_SetItemString(d, "units",
        PyUnicode_FromString(def.units.c_str()));
    PyDict_SetItemString(d, "url",
        PyUnicode_FromString(def.url.c_str()));
    PyDict_SetItemString(d, "description",
        PyUnicode_FromString(def.description.c_str()));
    PyDict_SetItemString(d, "minimum",
        def.minimum.has_value()
            ? PyFloat_FromDouble(*def.minimum)
            : (Py_INCREF(Py_None), Py_None));
    PyDict_SetItemString(d, "maximum",
        def.maximum.has_value()
            ? PyFloat_FromDouble(*def.maximum)
            : (Py_INCREF(Py_None), Py_None));
    return d;
}

// ---------------------------------------------------------------------------
// Helper — convert a MaterialModel to a Python dict (for Models property)
// ---------------------------------------------------------------------------

static PyObject* modelToDict(const MaterialModel& model)
{
    PyObject* d = PyDict_New();
    PyDict_SetItemString(d, "name",
        PyUnicode_FromString(model.getName().c_str()));
    PyDict_SetItemString(d, "uuid",
        PyUnicode_FromString(model.getUuid().c_str()));
    PyDict_SetItemString(d, "url",
        PyUnicode_FromString(model.getUrl().c_str()));
    PyDict_SetItemString(d, "description",
        PyUnicode_FromString(model.getDescription().c_str()));
    PyDict_SetItemString(d, "doi",
        PyUnicode_FromString(model.getDoi().c_str()));
    PyDict_SetItemString(d, "composite",
        PyBool_FromLong(model.isComposite() ? 1 : 0));
    return d;
}

// ---------------------------------------------------------------------------
// Representation
// ---------------------------------------------------------------------------

std::string ModelStorePy::representation() const
{
    std::ostringstream str;
    str << "<ModelStore at " << getModelStorePtr() << ">";
    return str.str();
}

// ---------------------------------------------------------------------------
// Constructor — returns the process-wide singleton
// ---------------------------------------------------------------------------

PyObject* ModelStorePy::PyMake(struct _typeobject*, PyObject*, PyObject*)
{
    return new ModelStorePy(&modelStore());
}

int ModelStorePy::PyInit(PyObject* /*args*/, PyObject* /*kwd*/)
{
    return 0;
}

// ---------------------------------------------------------------------------
// Properties
// ---------------------------------------------------------------------------

Py::Dict ModelStorePy::getModels() const
{
    Py::Dict dict;
    for (const auto* model : getModelStorePtr()->allModels()) {
        PyObject* d = modelToDict(*model);
        dict.setItem(Py::String(model->getUuid()), Py::Object(d, true));
    }
    return dict;
}

// ---------------------------------------------------------------------------
// Methods
// ---------------------------------------------------------------------------

PyObject* ModelStorePy::getModel(PyObject* args)
{
    const char* uuid = nullptr;
    if (!PyArg_ParseTuple(args, "s", &uuid))
        return nullptr;

    const MaterialModel* model = getModelStorePtr()->findByUuid(uuid);
    if (!model) {
        PyErr_SetString(PyExc_LookupError, "Model not found");
        return nullptr;
    }
    return modelToDict(*model);
}

PyObject* ModelStorePy::getModelByName(PyObject* args)
{
    const char* name = nullptr;
    if (!PyArg_ParseTuple(args, "s", &name))
        return nullptr;

    const MaterialModel* model = getModelStorePtr()->findByName(name);
    if (!model) {
        PyErr_SetString(PyExc_LookupError, "Model not found");
        return nullptr;
    }
    return modelToDict(*model);
}

PyObject* ModelStorePy::resolvedProperties(PyObject* args)
{
    const char* uuid = nullptr;
    if (!PyArg_ParseTuple(args, "s", &uuid))
        return nullptr;

    auto props = getModelStorePtr()->resolvedProperties(uuid);
    PyObject* dict = PyDict_New();
    for (const auto& [key, def] : props)
        PyDict_SetItemString(dict, key.c_str(), propDefToDict(def));
    return dict;
}

PyObject* ModelStorePy::resolvedPropertiesForModels(PyObject* args)
{
    PyObject* listObj = nullptr;
    if (!PyArg_ParseTuple(args, "O!", &PyList_Type, &listObj))
        return nullptr;

    std::vector<std::string> uuids;
    Py_ssize_t n = PyList_Size(listObj);
    for (Py_ssize_t i = 0; i < n; ++i) {
        PyObject* item = PyList_GetItem(listObj, i);
        if (!PyUnicode_Check(item)) {
            PyErr_SetString(PyExc_TypeError, "List must contain strings");
            return nullptr;
        }
        uuids.emplace_back(PyUnicode_AsUTF8(item));
    }

    auto props = getModelStorePtr()->resolvedPropertiesForModels(uuids);
    PyObject* dict = PyDict_New();
    for (const auto& [key, def] : props)
        PyDict_SetItemString(dict, key.c_str(), propDefToDict(def));
    return dict;
}

PyObject* ModelStorePy::allModels(PyObject* /*args*/)
{
    Py::List list;
    for (const auto* model : getModelStorePtr()->allModels()) {
        PyObject* d = modelToDict(*model);
        list.append(Py::Object(d, true));
    }
    return Py::new_reference_to(list);
}

PyObject* ModelStorePy::getCustomAttributes(const char* /*attr*/) const
{
    return nullptr;
}

int ModelStorePy::setCustomAttributes(const char* /*attr*/, PyObject* /*obj*/)
{
    return 0;
}
