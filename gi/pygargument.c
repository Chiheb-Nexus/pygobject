/* -*- Mode: C; c-basic-offset: 4 -*-
 * vim: tabstop=4 shiftwidth=4 expandtab
 *
 * Copyright (C) 2005-2009 Johan Dahlin <johan@gnome.org>
 *
 *   pygargument.c: GArgument - PyObject conversion fonctions.
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

#include <string.h>
#include <pygobject.h>


GQuark
pyg_argument_from_pyobject_error_quark(void)
{
  return g_quark_from_static_string ("pyg-argument-from-pyobject-quark");
}

gboolean
pyg_argument_from_pyobject_check(PyObject *object, GITypeInfo *type_info, GError **error)
{
    gboolean retval;
    GITypeTag type_tag;
    const gchar *py_type_name_expected;

    type_tag = g_type_info_get_tag(type_info);

    retval = TRUE;

    switch(type_tag) {
        case GI_TYPE_TAG_VOID:
            /* No check possible. */
            break;
        case GI_TYPE_TAG_BOOLEAN:
            /* No check; every Python object has a truth value. */
            break;
        case GI_TYPE_TAG_INT8:
        case GI_TYPE_TAG_INT16:
        case GI_TYPE_TAG_INT32:
        case GI_TYPE_TAG_INT:
        case GI_TYPE_TAG_SHORT:
        {
            long value;
            gint value_max, value_min;

            if (!(PyInt_Check(object) || PyLong_Check(object))) {
                py_type_name_expected = "int or long";
                goto check_error_type;
            }

            if (type_tag == GI_TYPE_TAG_INT8) {
                value_min = -128;
                value_max = 127;
            } else if (type_tag == GI_TYPE_TAG_INT16) {
                value_min = -32768;
                value_max = 32767;
            } else if (type_tag == GI_TYPE_TAG_INT32) {
                value_min = -2147483648;
                value_max = 2147483647;
            } else if (type_tag == GI_TYPE_TAG_INT) {
                value_min = G_MININT;
                value_max = G_MAXINT;
            } else if (type_tag == GI_TYPE_TAG_SHORT) {
                value_min = G_MINSHORT;
                value_max = G_MAXSHORT;
            } else {
                g_assert_not_reached();
                value_max = 0;
                value_min = 0;
            }

            value = PyInt_AsLong(object);
            if (PyErr_Occurred() || value < value_min || value > value_max) {
                PyErr_Clear();
                g_set_error(error, PyG_ARGUMENT_FROM_PYOBJECT_ERROR, PyG_ARGUMENT_FROM_PYOBJECT_ERROR_VALUE,
                        "Must range from %i to %i", value_min, value_max);
                retval = FALSE;
            }
            break;
        }
        case GI_TYPE_TAG_UINT8:
        case GI_TYPE_TAG_UINT16:
        case GI_TYPE_TAG_UINT32:
        case GI_TYPE_TAG_UINT:
        case GI_TYPE_TAG_USHORT:
        {
            guint value_max;

            if (type_tag == GI_TYPE_TAG_UINT8) {
                value_max = 255;
            } else if (type_tag == GI_TYPE_TAG_UINT16) {
                value_max = 65535;
            } else if (type_tag == GI_TYPE_TAG_UINT32) {
                value_max = 4294967295;
            } else if (type_tag == GI_TYPE_TAG_UINT) {
                value_max = G_MAXUINT;
            } else if (type_tag == GI_TYPE_TAG_USHORT) {
                value_max = G_MAXUSHORT;
            } else {
                g_assert_not_reached();
                value_max = 0;
            }

            if (PyInt_Check(object)) {
                long value = PyInt_AsLong(object);
                if (PyErr_Occurred() || value < 0 || value > value_max) {
                    PyErr_Clear();
                    g_set_error(error, PyG_ARGUMENT_FROM_PYOBJECT_ERROR, PyG_ARGUMENT_FROM_PYOBJECT_ERROR_VALUE,
                            "Must range from 0 to %u", value_max);
                    retval = FALSE;
                }
            } else if (PyLong_Check(object)) {
                unsigned long value = PyLong_AsUnsignedLong(object);
                if (PyErr_Occurred() || value > value_max) {
                    PyErr_Clear();
                    g_set_error(error, PyG_ARGUMENT_FROM_PYOBJECT_ERROR, PyG_ARGUMENT_FROM_PYOBJECT_ERROR_VALUE,
                            "Must range from 0 to %u", value_max);
                    retval = FALSE;
                }
            } else {
                py_type_name_expected = "int or long";
                goto check_error_type;
            }
            break;
        }
        case GI_TYPE_TAG_INT64:
        case GI_TYPE_TAG_LONG:
        case GI_TYPE_TAG_SSIZE:
            if (PyLong_Check(object)) {
                gint64 value_min, value_max;

                PyErr_Clear();
                if (type_tag == GI_TYPE_TAG_INT64) {
                    (void) PyLong_AsLongLong(object);
                    value_min = -9223372036854775808u;
                    value_max = 9223372036854775807;
                } else {
                    (void) PyLong_AsLong(object);

                    /* Could be different from above on a 64 bit arch. */
                    value_min = G_MINLONG;
                    value_max = G_MAXLONG;
                }
                if (PyErr_Occurred()) {
                    PyErr_Clear();
                    g_set_error(error, PyG_ARGUMENT_FROM_PYOBJECT_ERROR, PyG_ARGUMENT_FROM_PYOBJECT_ERROR_VALUE,
                            "Must range from %" G_GINT64_FORMAT " to %" G_GINT64_FORMAT, value_min, value_max);
                    retval = FALSE;
                }
            } else if (!PyInt_Check(object)) {
                /* Python Integer objects are implemented with longs, so no possible error if it is one. */
                py_type_name_expected = "int or long";
                goto check_error_type;
            }
            break;
        case GI_TYPE_TAG_UINT64:
        case GI_TYPE_TAG_ULONG:
        case GI_TYPE_TAG_SIZE:
        {
            guint64 value_max;

            if (type_tag == GI_TYPE_TAG_INT64) {
                value_max = 18446744073709551615u;
            } else {
                /* Could be different from above on a 64 bit arch. */
                value_max = G_MAXULONG;
            }

            if (PyLong_Check(object)) {
                PyErr_Clear();
                if (type_tag == GI_TYPE_TAG_UINT64) {
                    (void) PyLong_AsUnsignedLongLong(object);
                } else {
                    (void) PyLong_AsUnsignedLong(object);
                }
                if (PyErr_Occurred()) {
                    PyErr_Clear();
                    g_set_error(error, PyG_ARGUMENT_FROM_PYOBJECT_ERROR, PyG_ARGUMENT_FROM_PYOBJECT_ERROR_VALUE,
                            "Must range from 0 to %" G_GUINT64_FORMAT, value_max);
                    retval = FALSE;
                }
            } else if (PyInt_Check(object)) {
                long value;
                value = PyInt_AsLong(object);
                if (PyErr_Occurred() || value < 0) {
                    PyErr_Clear();
                    g_set_error(error, PyG_ARGUMENT_FROM_PYOBJECT_ERROR, PyG_ARGUMENT_FROM_PYOBJECT_ERROR_VALUE,
                            "Must range from 0 to %" G_GUINT64_FORMAT, value_max);
                    retval = FALSE;
                }
            } else {
                py_type_name_expected = "int or long";
                goto check_error_type;
            }
            break;
        }
        case GI_TYPE_TAG_FLOAT:
        case GI_TYPE_TAG_DOUBLE:
        {
            gdouble value;

            if (!PyFloat_Check(object)) {
                py_type_name_expected = "float";
                goto check_error_type;
            }

            value = PyFloat_AsDouble(object);
            if (type_tag == GI_TYPE_TAG_FLOAT) {
                if (PyErr_Occurred() || value < -G_MAXFLOAT || value > G_MAXFLOAT) {
                    PyErr_Clear();
                    g_set_error(error, PyG_ARGUMENT_FROM_PYOBJECT_ERROR, PyG_ARGUMENT_FROM_PYOBJECT_ERROR_VALUE,
                            "Must range from %f to %f", -G_MAXFLOAT, G_MAXFLOAT);
                    retval = FALSE;
                }
            } else if (type_tag == GI_TYPE_TAG_DOUBLE) {
                if (PyErr_Occurred()) {
                    PyErr_Clear();
                    g_set_error(error, PyG_ARGUMENT_FROM_PYOBJECT_ERROR, PyG_ARGUMENT_FROM_PYOBJECT_ERROR_VALUE,
                            "Must range from %f to %f", -G_MAXDOUBLE, G_MAXDOUBLE);
                    retval = FALSE;
                }
            }
            break;
        }
        case GI_TYPE_TAG_UTF8:
            if (!PyString_Check(object)) {
                py_type_name_expected = "string";
                goto check_error_type;
            }
            break;
        case GI_TYPE_TAG_ARRAY:
        {
            gssize size;
            gssize required_size;
            Py_ssize_t object_size;
            GITypeInfo *item_type_info;
            gsize i;

            if (!PyTuple_Check(object)) {
                py_type_name_expected = "tuple";
                goto check_error_type;
            }

            size = g_type_info_get_array_fixed_size(type_info);
            required_size = g_type_info_get_array_fixed_size(type_info);
            object_size = PyTuple_Size(object);
            if (required_size != -1 && object_size != required_size) {
                g_set_error(error, PyG_ARGUMENT_FROM_PYOBJECT_ERROR, PyG_ARGUMENT_FROM_PYOBJECT_ERROR_SIZE,
                        "Must contain %zd items, not %zd", required_size, object_size);
                retval = FALSE;
                break;
            }

            item_type_info = g_type_info_get_param_type(type_info, 0);

            for (i = 0; i < object_size; i++) {
                PyObject *item;

                item = PyTuple_GetItem(object, i);
                g_assert(item != NULL);

                g_assert(error == NULL || *error == NULL);
                if (!pyg_argument_from_pyobject_check(item, item_type_info, error)) {
                    g_prefix_error(error, "Item %zu: ", i);
                    retval = FALSE;
                    break;
                }
            }

            g_base_info_unref((GIBaseInfo *)item_type_info);

            break;
        }
        case GI_TYPE_TAG_INTERFACE:
        {
            GIBaseInfo *interface_info;
            GIInfoType interface_info_type;

            interface_info = g_type_info_get_interface(type_info);
            interface_info_type = g_base_info_get_type(interface_info);

            switch (interface_info_type) {
                case GI_INFO_TYPE_ENUM:
                {
                    (void) PyInt_AsLong(object);
                    if (PyErr_Occurred()) {
                        PyErr_Clear();
                        py_type_name_expected = "int";
                        goto check_error_type;
                    }
                    /* XXX: What if the value doesn't correspond to any enum field? */
                    break;
                }
                case GI_INFO_TYPE_STRUCT:
                case GI_INFO_TYPE_BOXED:
                {
                    GType gtype;

                    gtype = g_registered_type_info_get_g_type((GIRegisteredTypeInfo *)interface_info);

                    if (g_type_is_a(gtype, G_TYPE_VALUE)) {
                        /* Nothing to check. */
                        break;
                    } else if (g_type_is_a(gtype, G_TYPE_CLOSURE)) {
                        if (!PyCallable_Check(object)) {
                            g_base_info_unref(interface_info);
                            py_type_name_expected = "callable";
                            goto check_error_type;
                        }
                    } else {
                        GIBaseInfo *info;

                        info = pyg_base_info_from_object(object);
                        if (info == NULL || !g_base_info_equals(info, interface_info)) {
                            py_type_name_expected = g_base_info_get_name(interface_info);
                            if (info != NULL) {
                                g_base_info_unref(info);
                            }
                            g_base_info_unref(interface_info);
                            goto check_error_type;
                        }

                        g_base_info_unref(info);
                    }

                    break;
                }
                default:
                    /* TODO: To complete with other types. */
                    g_assert_not_reached();
            }

            g_base_info_unref(interface_info);

            break;
        }
        case GI_TYPE_TAG_GTYPE:
        {
            GType gtype;
            gtype = pyg_type_from_object(object);
            if (gtype == 0) {
                py_type_name_expected = "GType";
                goto check_error_type;
            }
            break;
        }
        case GI_TYPE_TAG_TIME_T:
        case GI_TYPE_TAG_FILENAME:
        case GI_TYPE_TAG_GLIST:
        case GI_TYPE_TAG_GSLIST:
        case GI_TYPE_TAG_GHASH:
        case GI_TYPE_TAG_ERROR:
            /* TODO */
        default:
            g_assert_not_reached();
    }

    g_assert(error == NULL || (retval == (*error == NULL)));
    return retval;

check_error_type:
    {
        PyObject *py_type;

        py_type = PyObject_Type(object);

        g_assert(py_type_name_expected != NULL);

        g_set_error(error, PyG_ARGUMENT_FROM_PYOBJECT_ERROR, PyG_ARGUMENT_FROM_PYOBJECT_ERROR_TYPE,
                "Must be %s, not %s", py_type_name_expected, ((PyTypeObject *)py_type)->tp_name);
        Py_XDECREF(py_type);
        return FALSE;
    }
}

GArgument
pyg_argument_from_pyobject(PyObject *object, GITypeInfo *type_info)
{
    GArgument arg;
    GITypeTag type_tag;

    type_tag = g_type_info_get_tag((GITypeInfo*)type_info);
    switch (type_tag) {
    case GI_TYPE_TAG_VOID:
        /* Nothing to do */
        break;
    case GI_TYPE_TAG_UTF8:
        if (object == Py_None)
            arg.v_pointer = NULL;
        else
            arg.v_pointer = g_strdup(PyString_AsString(object));
        break;
    case GI_TYPE_TAG_USHORT:
        arg.v_ushort = PyInt_AsLong(object);
        break;
    case GI_TYPE_TAG_UINT8:
        arg.v_uint8 = PyInt_AsLong(object);
        break;
    case GI_TYPE_TAG_UINT:
        arg.v_uint = PyInt_AsLong(object);
        break;
    case GI_TYPE_TAG_UINT16:
        arg.v_uint16 = PyInt_AsLong(object);
        break;
    case GI_TYPE_TAG_UINT32:
        arg.v_uint32 = PyLong_AsLongLong(object);
        break;
    case GI_TYPE_TAG_UINT64:
        if (PyInt_Check(object)) {
            PyObject *long_obj = PyNumber_Long(object);
            arg.v_uint64 = PyLong_AsUnsignedLongLong(long_obj);
            Py_DECREF(long_obj);
        } else
            arg.v_uint64 = PyLong_AsUnsignedLongLong(object);
        break;
    case GI_TYPE_TAG_SHORT:
        arg.v_short = PyInt_AsLong(object);
        break;
    case GI_TYPE_TAG_INT8:
        arg.v_int8 = PyInt_AsLong(object);
        break;
    case GI_TYPE_TAG_INT:
        arg.v_int = PyInt_AsLong(object);
        break;
    case GI_TYPE_TAG_SSIZE:
    case GI_TYPE_TAG_LONG:
        arg.v_long = PyInt_AsLong(object);
        break;
    case GI_TYPE_TAG_SIZE:
    case GI_TYPE_TAG_ULONG:
        arg.v_ulong = PyInt_AsLong(object);
        break;
    case GI_TYPE_TAG_BOOLEAN:
        arg.v_boolean = PyInt_AsLong(object);
        break;
    case GI_TYPE_TAG_INT16:
        arg.v_int16 = PyInt_AsLong(object);
        break;
    case GI_TYPE_TAG_INT32:
        arg.v_int32 = PyInt_AsLong(object);
        break;
    case GI_TYPE_TAG_INT64:
        arg.v_int64 = PyLong_AsLongLong(object);
        break;
    case GI_TYPE_TAG_FLOAT:
        arg.v_float = (float)PyFloat_AsDouble(object);
        break;
    case GI_TYPE_TAG_DOUBLE:
        arg.v_double = PyFloat_AsDouble(object);
        break;
    case GI_TYPE_TAG_INTERFACE:
    {
        GIBaseInfo* interface_info;
        GIInfoType interface_info_type;

        interface_info = g_type_info_get_interface(type_info);
        interface_info_type = g_base_info_get_type(interface_info);

        switch (interface_info_type) {
            case GI_INFO_TYPE_ENUM:
                arg.v_int = PyInt_AsLong(object);
                break;
            case GI_INFO_TYPE_STRUCT:
            {
                GType gtype;
                PyObject *py_buffer;

                gtype = g_registered_type_info_get_g_type((GIRegisteredTypeInfo *)interface_info);

                if (g_type_is_a(gtype, G_TYPE_VALUE)) {
                    GValue *value;
                    int retval;
                    PyObject *py_type;

                    value = g_slice_new0(GValue);

                    py_type = PyObject_Type(object);
                    g_assert(py_type != NULL);

                    g_value_init(value, pyg_type_from_object(py_type));

                    retval = pyg_value_from_pyobject(value, object);
                    g_assert(retval == 0);

                    arg.v_pointer = value;
                    break;
                } else if (g_type_is_a(gtype, G_TYPE_CLOSURE)) {
                    arg.v_pointer = pyg_closure_new(object, NULL, NULL);
                    break;
                }

                py_buffer = PyObject_GetAttrString(object, "__buffer__");
                g_assert(py_buffer != NULL);
                (*py_buffer->ob_type->tp_as_buffer->bf_getreadbuffer)(py_buffer, 0, &arg.v_pointer);

                break;
            }
            case GI_INFO_TYPE_OBJECT:
                if (object == Py_None) {
                    arg.v_pointer = NULL;
                    break;
                }
                arg.v_pointer = pygobject_get(object);
                break;
            default:
                /* TODO: To complete with other types. */
                g_assert_not_reached();
        }
        g_base_info_unref((GIBaseInfo *)interface_info);
        break;
    }
    case GI_TYPE_TAG_ARRAY:
    {
        gsize length;
        arg.v_pointer = pyg_array_from_pyobject(object, type_info, &length);
        break;
    }
    case GI_TYPE_TAG_ERROR:
        /* Allow NULL GError, otherwise fall through */
        if (object == Py_None) {
            arg.v_pointer = NULL;
            break;
        }
    case GI_TYPE_TAG_GTYPE:
        arg.v_int = pyg_type_from_object(object);
        break;
    default:
        g_print("<PyO->GArg> GITypeTag %s is unhandled\n",
                g_type_tag_to_string(type_tag));
        break;
    }

    return arg;
}

static PyObject *
glist_to_pyobject(GITypeTag list_tag, GITypeInfo *type_info, GList *list, GSList *slist)
{
    PyObject *py_list;
    int i;
    GArgument arg;
    PyObject *child_obj;

    if ((py_list = PyList_New(0)) == NULL) {
        g_list_free(list);
        return NULL;
    }
    i = 0;
    if (list_tag == GI_TYPE_TAG_GLIST) {
        for ( ; list != NULL; list = list->next) {
            arg.v_pointer = list->data;

            child_obj = pyg_argument_to_pyobject(&arg, type_info);

            if (child_obj == NULL) {
                g_list_free(list);
                Py_DECREF(py_list);
                return NULL;
            }
            PyList_Append(py_list, child_obj);
            Py_DECREF(child_obj);

            ++i;
        }
    } else {
        for ( ; slist != NULL; slist = slist->next) {
            arg.v_pointer = slist->data;

            child_obj = pyg_argument_to_pyobject(&arg, type_info);

            if (child_obj == NULL) {
                g_list_free(list);
                Py_DECREF(py_list);
                return NULL;
            }
            PyList_Append(py_list, child_obj);
            Py_DECREF(child_obj);

            ++i;
        }
    }
    g_list_free(list);
    return py_list;
}

static
gsize
pyg_type_get_size(GITypeTag type_tag)
{
    gsize size;

    switch(type_tag) {
        case GI_TYPE_TAG_VOID:
            size = sizeof(void);
            break;
        case GI_TYPE_TAG_BOOLEAN:
            size = sizeof(gboolean);
            break;
        case GI_TYPE_TAG_INT:
        case GI_TYPE_TAG_UINT:
            size = sizeof(gint);
            break;
        case GI_TYPE_TAG_INT8:
        case GI_TYPE_TAG_UINT8:
            size = sizeof(gint8);
            break;
        case GI_TYPE_TAG_INT16:
        case GI_TYPE_TAG_UINT16:
            size = sizeof(gint16);
            break;
        case GI_TYPE_TAG_INT32:
        case GI_TYPE_TAG_UINT32:
            size = sizeof(gint32);
            break;
        case GI_TYPE_TAG_INT64:
        case GI_TYPE_TAG_UINT64:
            size = sizeof(gint64);
            break;
        case GI_TYPE_TAG_LONG:
        case GI_TYPE_TAG_ULONG:
            size = sizeof(glong);
            break;
        case GI_TYPE_TAG_SIZE:
        case GI_TYPE_TAG_SSIZE:
            size = sizeof(gsize);
            break;
        case GI_TYPE_TAG_FLOAT:
            size = sizeof(gfloat);
            break;
        case GI_TYPE_TAG_DOUBLE:
            size = sizeof(gdouble);
            break;
        case GI_TYPE_TAG_UTF8:
            size = sizeof(gchar *);
            break;
        default:
            /* TODO: Complete with other types */
            g_assert_not_reached();
    }

    return size;
}

gpointer
pyg_array_from_pyobject(PyObject *object, GITypeInfo *type_info, gsize *length)
{
    gpointer items;
    gpointer current_item;
    gssize item_size;
    gboolean is_zero_terminated;
    GITypeInfo *item_type_info;
    GITypeTag type_tag;
    gsize i;

    is_zero_terminated = g_type_info_is_zero_terminated(type_info);
    item_type_info = g_type_info_get_param_type(type_info, 0);

    type_tag = g_type_info_get_tag(item_type_info);
    item_size = pyg_type_get_size(type_tag);

    *length = PyTuple_Size(object);
    items = g_try_malloc(*length * (item_size + (is_zero_terminated ? 1 : 0)));

    if (items == NULL) {
        g_base_info_unref((GIBaseInfo *)item_type_info);
        return NULL;
    }

    current_item = items;
    for (i = 0; i < *length; i++) {
        GArgument arg;
        PyObject *py_item;

        py_item = PyTuple_GetItem(object, i);
        g_assert(py_item != NULL);

        arg = pyg_argument_from_pyobject(py_item, item_type_info);

        g_memmove(current_item, &arg, item_size);

        current_item += item_size;
    }

    if (is_zero_terminated) {
        memset(current_item, 0, item_size);
    }

    g_base_info_unref((GIBaseInfo *)item_type_info);

    return items;
}

PyObject *
pyg_array_to_pyobject(gpointer items, gsize length, GITypeInfo *type_info)
{
    PyObject *py_items;
    gsize item_size;
    GITypeInfo *item_type_info;
    GITypeTag type_tag;
    gsize i;
    gpointer current_item;

    if (g_type_info_is_zero_terminated(type_info)) {
        length = g_strv_length(items);
    }

    py_items = PyTuple_New(length);
    if (py_items == NULL) {
        return NULL;
    }

    item_type_info = g_type_info_get_param_type (type_info, 0);
    type_tag = g_type_info_get_tag(item_type_info);
    item_size = pyg_type_get_size(type_tag);

    current_item = items;
    for(i = 0; i < length; i++) {
        PyObject *item;
        int retval;

        item = pyg_argument_to_pyobject((GArgument *)current_item, item_type_info);
        if (item == NULL) {
            g_base_info_unref((GIBaseInfo *)item_type_info);
            Py_DECREF(py_items);
            return NULL;
        }

        retval = PyTuple_SetItem(py_items, i, item);
        if (retval) {
            g_base_info_unref((GIBaseInfo *)item_type_info);
            Py_DECREF(py_items);
            return NULL;
        }

        current_item += item_size;
    }

    return py_items;
}

PyObject *
pyg_argument_to_pyobject(GArgument *arg, GITypeInfo *type_info)
{
    GITypeTag type_tag;
    PyObject *obj;
    GITypeInfo *param_info;

    g_return_val_if_fail(type_info != NULL, NULL);
    type_tag = g_type_info_get_tag(type_info);

    obj = NULL;

    switch (type_tag) {
    case GI_TYPE_TAG_VOID:
        // TODO: Should we take this as a buffer?
        g_warning("pybank doesn't know what to do with void types");
        obj = Py_None;
        break;
    case GI_TYPE_TAG_GLIST:
    case GI_TYPE_TAG_GSLIST:
        param_info = g_type_info_get_param_type(type_info, 0);
        g_assert(param_info != NULL);
        obj = glist_to_pyobject(type_tag,
                                param_info,
                                type_tag == GI_TYPE_TAG_GLIST ? arg->v_pointer : NULL,
                                type_tag == GI_TYPE_TAG_GSLIST ? arg->v_pointer : NULL);
        break;
    case GI_TYPE_TAG_BOOLEAN:
        obj = PyBool_FromLong(arg->v_boolean);
        break;
    case GI_TYPE_TAG_USHORT:
        obj = PyInt_FromLong(arg->v_ushort);
        break;
    case GI_TYPE_TAG_UINT8:
        obj = PyInt_FromLong(arg->v_uint8);
        break;
    case GI_TYPE_TAG_UINT:
        obj = PyInt_FromLong(arg->v_uint);
        break;
    case GI_TYPE_TAG_UINT16:
        obj = PyInt_FromLong(arg->v_uint16);
        break;
    case GI_TYPE_TAG_UINT32:
        obj = PyLong_FromLongLong(arg->v_uint32);
        break;
    case GI_TYPE_TAG_UINT64:
        obj = PyLong_FromUnsignedLongLong(arg->v_uint64);
        break;
    case GI_TYPE_TAG_SHORT:
        obj = PyInt_FromLong(arg->v_short);
        break;
    case GI_TYPE_TAG_INT:
        obj = PyInt_FromLong(arg->v_int);
        break;
    case GI_TYPE_TAG_LONG:
        obj = PyInt_FromLong(arg->v_long);
        break;
    case GI_TYPE_TAG_ULONG:
        obj = PyInt_FromLong(arg->v_ulong);
        break;
    case GI_TYPE_TAG_SSIZE:
        obj = PyInt_FromLong(arg->v_ssize);
        break;
    case GI_TYPE_TAG_SIZE:
        obj = PyInt_FromLong(arg->v_size);
        break;
    case GI_TYPE_TAG_INT8:
        obj = PyInt_FromLong(arg->v_int8);
        break;
    case GI_TYPE_TAG_INT16:
        obj = PyInt_FromLong(arg->v_int16);
        break;
    case GI_TYPE_TAG_INT32:
        obj = PyInt_FromLong(arg->v_int32);
        break;
    case GI_TYPE_TAG_INT64:
        obj = PyLong_FromLongLong(arg->v_int64);
        break;
    case GI_TYPE_TAG_FLOAT:
        obj = PyFloat_FromDouble(arg->v_float);
        break;
    case GI_TYPE_TAG_DOUBLE:
        obj = PyFloat_FromDouble(arg->v_double);
        break;
    case GI_TYPE_TAG_FILENAME:
    case GI_TYPE_TAG_UTF8:
        if (arg->v_string == NULL)
            obj = Py_None;
        else
            obj = PyString_FromString(arg->v_string);
        break;
    case GI_TYPE_TAG_INTERFACE:
    {
        GIBaseInfo* interface_info;
        GIInfoType interface_info_type;

        interface_info = g_type_info_get_interface(type_info);
        interface_info_type = g_base_info_get_type(interface_info);

        if (arg->v_pointer == NULL) {
            obj = Py_None;
        }

        switch (interface_info_type) {
            case GI_INFO_TYPE_ENUM:
               obj = PyInt_FromLong(arg->v_int);
                break;
            case GI_INFO_TYPE_STRUCT:
            {
                GType gtype;
                const gchar *module_name;
                const gchar *type_name;
                PyObject *module;
                PyObject *type;
                gsize size;
                PyObject *buffer;
                PyObject **dict;
                int retval;

                gtype = g_registered_type_info_get_g_type((GIRegisteredTypeInfo *)interface_info);

                if (g_type_is_a(gtype, G_TYPE_VALUE)) {
                    obj = pyg_value_as_pyobject(arg->v_pointer, FALSE);
                    g_value_unset(arg->v_pointer);
                    break;
                }

                /* Wrap the structure. */
                module_name = g_base_info_get_namespace(interface_info);
                type_name = g_base_info_get_name(interface_info);

                module = pygi_repository_get_py_module(module_name);
                if (module == NULL) {
                    PyErr_Format(PyExc_TypeError, "Type %s.%s not defined", module_name, type_name);
                    break;
                }

                type = PyObject_GetAttrString(module, type_name);
                if (type == NULL) {
                    PyErr_Format(PyExc_TypeError, "Type %s.%s not defined", module_name, type_name);
                    Py_DECREF(module);
                    break;
                }

                obj = PyObject_GC_New(PyObject, (PyTypeObject *)type);
                if (obj == NULL) {
                    Py_DECREF(type);
                    Py_DECREF(module);
                    break;
                }

                /* FIXME: Any better way to initialize the dict pointer? */
                dict = (PyObject **)((char *)obj + ((PyTypeObject *)type)->tp_dictoffset);
                *dict = NULL;

                size = g_struct_info_get_size ((GIStructInfo *)interface_info);
                buffer = PyBuffer_FromReadWriteMemory(arg->v_pointer, size);
                if (buffer == NULL) {
                    Py_DECREF(obj);
                    Py_DECREF(type);
                    Py_DECREF(module);
                    break;
                }

                retval = PyObject_SetAttrString(obj, "__buffer__", buffer);
                g_assert(retval == 0);

                break;
            }
            case GI_INFO_TYPE_OBJECT:
            {
                const gchar *module_name;
                const gchar *type_name;
                PyObject *type;
                PyObject *module;

                /* Make sure the class is initialized */
                module_name = g_base_info_get_namespace(interface_info);
                type_name = g_base_info_get_name(interface_info);

                module = pygi_repository_get_py_module(module_name);
                if (module == NULL) {
                    PyErr_Format(PyExc_TypeError, "Type %s.%s not defined", module_name, type_name);
                    break;
                }

                type = PyObject_GetAttrString(module, type_name);
                if (type == NULL) {
                    PyErr_Format(PyExc_TypeError, "Type %s.%s not defined", module_name, type_name);
                    break;
                }

                obj = pygobject_new(arg->v_pointer);

                break;
            }
            default:
                /* TODO: To complete with other types. */
                g_assert_not_reached();
        }

        g_base_info_unref((GIBaseInfo *)interface_info);

        break;
    }
    case GI_TYPE_TAG_ARRAY:
        g_warning("pyg_argument_to_pyobject: use pyarray_to_pyobject instead for arrays");
        obj = Py_None;
        break;
    case GI_TYPE_TAG_GTYPE:
    {
        GType gtype;
        gtype = arg->v_int;
        obj = pyg_type_wrapper_new(gtype);
        break;
    }
    default:
        g_print("<GArg->PyO> GITypeTag %s is unhandled\n",
                g_type_tag_to_string(type_tag));
        obj = PyString_FromString("<unhandled return value!>"); /*  */
        break;
    }

    Py_XINCREF(obj);
    return obj;
}

