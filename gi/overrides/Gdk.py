# -*- Mode: Python; py-indent-offset: 4 -*-
# vim: tabstop=4 shiftwidth=4 expandtab
#
# Copyright (C) 2009 Johan Dahlin <johan@gnome.org>
#               2010 Simon van der Linden <svdlinden@src.gnome.org>
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301
# USA

from ..overrides import override
from ..importer import modules

Gdk = modules['Gdk']._introspection_module

__all__ = []

class Color(Gdk.Color):

    def __init__(self, red, green, blue):
        Gdk.Color.__init__(self)
        self.red = red
        self.green = green
        self.blue = blue

    def __new__(cls, *args, **kwargs):
        return Gdk.Color.__new__(cls)

    def __repr__(self):
        return '<Gdk.Color(red=%d, green=%d, blue=%d)>' % (self.red, self.green, self.blue)

Color = override(Color)
__all__.append('Color')

if Gdk._version == '2.0':
    class Rectangle(Gdk.Rectangle):

        def __init__(self, x, y, width, height):
            Gdk.Rectangle.__init__(self)
            self.x = x
            self.y = y
            self.width = width
            self.height = height

        def __new__(cls, *args, **kwargs):
            return Gdk.Rectangle.__new__(cls)

        def __repr__(self):
            return '<Gdk.Rectangle(x=%d, y=%d, width=%d, height=%d)>' % (self.x, self.y, self.height, self.width)

    Rectangle = override(Rectangle)
    __all__.append('Rectangle')
else:
    from gi.repository import cairo as _cairo
    Rectangle = _cairo.RectangleInt

    __all__.append('Rectangle')

if Gdk._version == '2.0':
    class Drawable(Gdk.Drawable):
        def cairo_create(self):
            return Gdk.cairo_create(self)

    Drawable = override(Drawable)
    __all__.append('Drawable')
else:
    class Window(Gdk.Window):
        def __new__(cls, parent, attributes, attributes_mask):
            # Gdk.Window had to be made abstract,
            # this override allows using the standard constructor
            return Gdk.Window.new(parent, attributes, attributes_mask)
        def __init__(self, parent, attributes, attributes_mask):
            pass
        def cairo_create(self):
            return Gdk.cairo_create(self)

    Window = override(Window)
    __all__.append('Window')

Gdk.EventType._2BUTTON_PRESS = getattr(Gdk.EventType, "2BUTTON_PRESS")
Gdk.EventType._3BUTTON_PRESS = getattr(Gdk.EventType, "3BUTTON_PRESS")

class Event(Gdk.Event):
    _UNION_MEMBERS = {
        Gdk.EventType.DELETE: 'any',
        Gdk.EventType.DESTROY: 'any',
        Gdk.EventType.EXPOSE: 'expose',
        Gdk.EventType.MOTION_NOTIFY: 'motion',
        Gdk.EventType.BUTTON_PRESS: 'button',
        Gdk.EventType._2BUTTON_PRESS: 'button',
        Gdk.EventType._3BUTTON_PRESS: 'button',
        Gdk.EventType.BUTTON_RELEASE: 'button',
        Gdk.EventType.KEY_PRESS: 'key',
        Gdk.EventType.KEY_RELEASE: 'key',
        Gdk.EventType.ENTER_NOTIFY: 'crossing',
        Gdk.EventType.LEAVE_NOTIFY: 'crossing',
        Gdk.EventType.FOCUS_CHANGE: 'focus_change',
        Gdk.EventType.CONFIGURE: 'configure',
        Gdk.EventType.MAP: 'any',
        Gdk.EventType.UNMAP: 'any',
        Gdk.EventType.PROPERTY_NOTIFY: 'property',
        Gdk.EventType.SELECTION_CLEAR: 'selection',
        Gdk.EventType.SELECTION_REQUEST: 'selection',
        Gdk.EventType.SELECTION_NOTIFY: 'selection',
        Gdk.EventType.PROXIMITY_IN: 'proximity',
        Gdk.EventType.PROXIMITY_OUT: 'proximity',
        Gdk.EventType.DRAG_ENTER: 'dnd',
        Gdk.EventType.DRAG_LEAVE: 'dnd',
        Gdk.EventType.DRAG_MOTION: 'dnd',
        Gdk.EventType.DRAG_STATUS: 'dnd',
        Gdk.EventType.DROP_START: 'dnd',
        Gdk.EventType.DROP_FINISHED: 'dnd',
        Gdk.EventType.CLIENT_EVENT: 'client',
        Gdk.EventType.VISIBILITY_NOTIFY: 'visibility',
    }

    if Gdk._version == '2.0':
        _UNION_MEMBERS[Gdk.EventType.NO_EXPOSE] = 'no_expose'

    def __new__(cls, *args, **kwargs):
        return Gdk.Event.__new__(cls)

    def __getattr__(self, name):
        real_event = getattr(self, '_UNION_MEMBERS').get(self.type)
        if real_event:
            return getattr(getattr(self, real_event), name)
        else:
            raise AttributeError("'%s' object has no attribute '%s'" % (self.__class__.__name__, name))

Event = override(Event)
__all__.append('Event')

class DragContext(Gdk.DragContext):
    def finish(self, success, del_, time):
        Gtk = modules['Gtk']._introspection_module
        Gtk.drag_finish(self, success, del_, time)

DragContext = override(DragContext)
__all__.append('DragContext')

class Cursor(Gdk.Cursor):
    def __new__(cls, *args, **kwds):
        arg_len = len(args)
        kwd_len = len(kwds)
        total_len = arg_len + kwd_len

        def _new(cursor_type):
            return cls.new(cursor_type)

        def _new_for_display(display, cursor_type):
            return cls.new_for_display(display, cursor_type)

        def _new_from_pixbuf(display, pixbuf, x, y):
            return cls.new_from_pixbuf(display, pixbuf, x, y)

        def _new_from_pixmap(source, mask, fg, bg, x, y):
            return cls.new_from_pixmap(source, mask, fg, bg, x, y)

        _constructor = None
        if total_len == 1:
            _constructor = _new
        elif total_len == 2:
            _constructor = _new_for_display
        elif total_len == 4:
            _constructor = _new_from_pixbuf
        elif total_len == 6:
            if Gdk._version != '2.0':
                # pixmaps don't exist in Gdk 3.0
                raise ValueError("Wrong number of parameters")
            _constructor = _new_from_pixmap
        else:
            raise ValueError("Wrong number of parameters")

        return _constructor(*args, **kwds)

    def __init__(self, *args, **kwargs):
        Gdk.Cursor.__init__(self)

Cursor = override(Cursor)
__all__.append('Cursor')

import sys

initialized, argv = Gdk.init_check(sys.argv)
if not initialized:
    raise RuntimeError("Gdk couldn't be initialized")
