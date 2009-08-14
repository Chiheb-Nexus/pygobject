/* -*- Mode: C; c-basic-offset: 4 -*-
 * vim: tabstop=4 shiftwidth=4 expandtab
 *
 * Copyright (C) 2005-2009 Johan Dahlin <johan@gnome.org>
 *
 *   pygirepository.c: GIRepository wrapper.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301
 * USA
 */

#include "pygi-private.h"

PyObject *PyGIRepositoryError;

static PyMethodDef _PyGIRepository_methods[];

PyTypeObject PyGIRepository_Type = {
    PyObject_HEAD_INIT(NULL)
    0,
    "gi.Repository",         /* tp_name */
    sizeof(PyGIRepository),  /* tp_basicsize */
    0,                       /* tp_itemsize */
    (destructor)NULL,        /* tp_dealloc */
    (printfunc)NULL,         /* tp_print */
    (getattrfunc)NULL,       /* tp_getattr */
    (setattrfunc)NULL,       /* tp_setattr */
    (cmpfunc)NULL,           /* tp_compare */
    (reprfunc)NULL,          /* tp_repr */
    NULL,                    /* tp_as_number */
    NULL,                    /* tp_as_sequence */
    NULL,                    /* tp_as_mapping */
    (hashfunc)NULL,          /* tp_hash */
    (ternaryfunc)NULL,       /* tp_call */
    (reprfunc)NULL,          /* tp_str */
    (getattrofunc)NULL,      /* tp_getattro */
    (setattrofunc)NULL,      /* tp_setattro */
    NULL,                    /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,      /* tp_flags */
    NULL,                    /* tp_doc */
    (traverseproc)NULL,      /* tp_traverse */
    (inquiry)NULL,           /* tp_clear */
    (richcmpfunc)NULL,       /* tp_richcompare */
    0,                       /* tp_weaklistoffset */
    (getiterfunc)NULL,       /* tp_iter */
    (iternextfunc)NULL,      /* tp_iternext */
    _PyGIRepository_methods, /* tp_methods */
};

static PyObject *
_wrap_g_irepository_get_default(PyObject *self)
{
    static PyGIRepository *repository = NULL;

    if (!repository) {
        repository = (PyGIRepository *)PyObject_New(PyGIRepository, &PyGIRepository_Type);
        if (repository == NULL) {
            return NULL;
        }

        repository->repository = g_irepository_get_default();
    }

    Py_INCREF((PyObject *)repository);
    return (PyObject *)repository;
}

static PyObject *
_wrap_g_irepository_require(PyGIRepository *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "namespace", "version", "lazy", NULL };

    const char *namespace_;
    const char *version = NULL;
    PyObject *lazy = NULL;
    GIRepositoryLoadFlags flags = 0;
    GTypelib *typelib;
    GError *error;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "s|sO:Repository.require",
            kwlist, &namespace_, &version, &lazy)) {
        return NULL;
    }

    if (lazy != NULL && PyObject_IsTrue(lazy)) {
        flags |= G_IREPOSITORY_LOAD_FLAG_LAZY;
    }

    error = NULL;
    typelib = g_irepository_require(self->repository, namespace_, version, flags, &error);
    if (error != NULL) {
        PyErr_SetString(PyGIRepositoryError, error->message);
        g_error_free(error);
        return NULL;
    }

    Py_RETURN_NONE;
}

static PyObject *
_wrap_g_irepository_find_by_name(PyGIRepository *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "namespace", "name", NULL };

    const char *namespace_;
    const char *name;
    GIBaseInfo *info;
    PyObject *py_info;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
            "ss:Repository.find_by_name", kwlist, &namespace_, &name)) {
        return NULL;
    }

    info = g_irepository_find_by_name(self->repository, namespace_, name);
    if (info == NULL) {
        Py_RETURN_NONE;
    }

    py_info = pyg_info_new(info);

    g_base_info_unref(info);

    return py_info;
}

static PyObject *
_wrap_g_irepository_get_namespaces(PyGIRepository *self)
{
    char ** namespaces;
    int i, length;
    PyObject *retval;

    namespaces = g_irepository_get_loaded_namespaces(self->repository);

    length = g_strv_length(namespaces);
    retval = PyTuple_New(length);

    for (i = 0; i < length; i++)
	PyTuple_SetItem(retval, i, PyString_FromString(namespaces[i]));

    g_strfreev (namespaces);

    return retval;
}

static PyObject *
_wrap_g_irepository_get_infos(PyGIRepository *self,
			      PyObject *args,
			      PyObject *kwargs)
{
    static char *kwlist[] = { "namespace", NULL };
    char *namespace;
    int i, length;
    PyObject *retval;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
				     "s:Repository.get_infos",
				     kwlist, &namespace))
        return NULL;

    length = g_irepository_get_n_infos(self->repository, namespace);

    retval = PyTuple_New(length);

    for (i = 0; i < length; i++) {
	GIBaseInfo *info = g_irepository_get_info(self->repository, namespace, i);
	PyTuple_SetItem(retval, i, pyg_info_new(info));
    }

    return retval;
}

static PyObject *
_wrap_g_irepository_is_registered(PyGIRepository *self,
				  PyObject *args,
				  PyObject *kwargs)
{
    static char *kwlist[] = { "namespace", NULL };
    char *namespace;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
				     "s:Repository.is_registered",
				     kwlist, &namespace))
        return NULL;

    return PyBool_FromLong(g_irepository_is_registered(self->repository, namespace, NULL));
}

static PyObject *
_wrap_g_irepository_get_c_prefix(PyGIRepository *self,
				 PyObject *args,
				 PyObject *kwargs)
{
    static char *kwlist[] = { "namespace", NULL };
    char *namespace;


    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
				     "s:Repository.get_c_prefix",
				     kwlist, &namespace))
        return NULL;

    return PyString_FromString(g_irepository_get_c_prefix(self->repository, namespace));
}

static PyObject *
_wrap_g_irepository_get_typelib_path(PyGIRepository *self,
                                     PyObject *args,
                                     PyObject *kwargs)
{
    static char *kwlist[] = { "namespace", NULL };
    gchar *namespace_;
    const gchar *typelib_path;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
            "s:Repository.get_typelib_path", kwlist, &namespace_)) {
        return NULL;
    }

    typelib_path = g_irepository_get_typelib_path(self->repository, namespace_);
    if (typelib_path == NULL) {
        PyErr_Format(PyExc_RuntimeError, "Namespace '%s' not loaded", namespace_);
        return NULL;
    }

    return PyString_FromString(typelib_path);
}

static PyMethodDef _PyGIRepository_methods[] = {
    { "get_default", (PyCFunction)_wrap_g_irepository_get_default, METH_STATIC|METH_NOARGS },
    { "require", (PyCFunction)_wrap_g_irepository_require, METH_VARARGS|METH_KEYWORDS },
    { "get_namespaces", (PyCFunction)_wrap_g_irepository_get_namespaces, METH_NOARGS },
    { "get_infos", (PyCFunction)_wrap_g_irepository_get_infos, METH_VARARGS|METH_KEYWORDS },
    { "find_by_name", (PyCFunction)_wrap_g_irepository_find_by_name, METH_VARARGS|METH_KEYWORDS },
    { "is_registered", (PyCFunction)_wrap_g_irepository_is_registered, METH_VARARGS|METH_KEYWORDS },
    { "get_c_prefix", (PyCFunction)_wrap_g_irepository_get_c_prefix, METH_VARARGS|METH_KEYWORDS },
    { "get_typelib_path", (PyCFunction)_wrap_g_irepository_get_typelib_path, METH_VARARGS|METH_KEYWORDS },
    { NULL, NULL, 0 }
};

void
pygi_repository_register_types(PyObject *m)
{
    PyGIRepository_Type.ob_type = &PyType_Type;
    if (PyType_Ready(&PyGIRepository_Type)) {
        return;
    }
    if (PyModule_AddObject(m, "Repository", (PyObject *)&PyGIRepository_Type)) {
        return;
    }

    PyGIRepositoryError = PyErr_NewException("gi.RepositoryError", NULL, NULL);
    if (PyModule_AddObject(m, "RepositoryError", PyGIRepositoryError)) {
        return;
    }
}

