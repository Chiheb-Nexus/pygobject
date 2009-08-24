/* -*- Mode: C; c-basic-offset: 4 -*-
 * vim: tabstop=4 shiftwidth=4 expandtab
 */
#ifndef __PYGI_PRIVATE_H__
#define __PYGI_PRIVATE_H__

#ifdef __PYGI_H__
#   error "Import pygi.h or pygi-private.h, but not both"
#endif

#ifdef HAVE_CONFIG_H
#   include <config.h>
#endif

#include <Python.h>

#include "pygi.h"

#include "pygi-repository.h"
#include "pygi-info.h"
#include "pygi-argument.h"

G_BEGIN_DECLS


/* Python types */

extern PyTypeObject PyGIRepository_Type;

extern PyTypeObject PyGIBaseInfo_Type;
extern PyTypeObject PyGICallableInfo_Type;
extern PyTypeObject PyGIFunctionInfo_Type;
extern PyTypeObject PyGIRegisteredTypeInfo_Type;
extern PyTypeObject PyGIStructInfo_Type;
extern PyTypeObject PyGIEnumInfo_Type;
extern PyTypeObject PyGIObjectInfo_Type;
extern PyTypeObject PyGIInterfaceInfo_Type;
extern PyTypeObject PyGIConstantInfo_Type;
extern PyTypeObject PyGIValueInfo_Type;
extern PyTypeObject PyGIFieldInfo_Type;
extern PyTypeObject PyGIUnresolvedInfo_Type;


/* Errors */

extern PyObject *PyGIRepositoryError;


/* Functions (defined in gimodule.c) */

PyObject* pygi_type_find_by_name (const char *namespace_,
                                  const char *name);
PyObject* pygi_type_find_by_gi_info (GIBaseInfo *info);

GIBaseInfo* pygi_object_get_gi_info (PyObject     *object,
                                     PyTypeObject *type);


/* Private */


#define _PyGI_ERROR_PREFIX(format, ...) G_STMT_START { \
    PyObject *py_error_prefix; \
    py_error_prefix = PyString_FromFormat(format, ## __VA_ARGS__); \
    if (py_error_prefix != NULL) { \
        PyObject *py_error_type, *py_error_value, *py_error_traceback; \
        PyErr_Fetch(&py_error_type, &py_error_value, &py_error_traceback); \
        if (PyString_Check(py_error_value)) { \
            PyString_ConcatAndDel(&py_error_prefix, py_error_value); \
            if (py_error_prefix != NULL) { \
                py_error_value = py_error_prefix; \
            } \
        } \
        PyErr_Restore(py_error_type, py_error_value, py_error_traceback); \
    } \
} G_STMT_END


/* GArray */

/* Redefine g_array_index because we want it to return the i-th element, casted
 * to the type t, of the array a, and not the i-th element of the array a casted to the type t. */
#define _g_array_index(a,t,i) \
    *(t *)((a)->data + g_array_get_element_size(a) * (i))


G_END_DECLS

#endif /* __PYGI_PRIVATE_H__ */
