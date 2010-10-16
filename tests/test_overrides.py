# -*- Mode: Python; py-indent-offset: 4 -*-
# vim: tabstop=4 shiftwidth=4 expandtab

import pygtk
pygtk.require("2.0")

import unittest

import sys
sys.path.insert(0, "../")

from gi.repository import GLib
from gi.repository import GObject
from gi.repository import Gdk
from gi.repository import Gtk
import gi.overrides as overrides

class TestGLib(unittest.TestCase):

    def test_gvariant(self):
        variant = GLib.Variant('i', 42)
        self.assertTrue(isinstance(variant, GLib.Variant))
        self.assertEquals(variant.get_int32(), 42)

        variant = GLib.Variant('(ss)', 'mec', 'mac')
        self.assertTrue(isinstance(variant, GLib.Variant))
        self.assertTrue(isinstance(variant.get_child_value(0), GLib.Variant))
        self.assertTrue(isinstance(variant.get_child_value(1), GLib.Variant))
        self.assertEquals(variant.get_child_value(0).get_string(), 'mec')
        self.assertEquals(variant.get_child_value(1).get_string(), 'mac')

        variant = GLib.Variant('a{si}', {'key1': 1, 'key2': 2})
        self.assertTrue(isinstance(variant, GLib.Variant))
        self.assertTrue(isinstance(variant.get_child_value(0), GLib.Variant))
        self.assertTrue(isinstance(variant.get_child_value(1), GLib.Variant))
        # Looks like order is not preserved
        self.assertEquals(variant.get_child_value(1).get_child_value(0).get_string(), 'key1')
        self.assertEquals(variant.get_child_value(1).get_child_value(1).get_int32(), 1)
        self.assertEquals(variant.get_child_value(0).get_child_value(0).get_string(), 'key2')
        self.assertEquals(variant.get_child_value(0).get_child_value(1).get_int32(), 2)

class TestGdk(unittest.TestCase):

    def test_color(self):
        color = Gdk.Color(100, 200, 300)
        self.assertEquals(color.red, 100)
        self.assertEquals(color.green, 200)
        self.assertEquals(color.blue, 300)

    def test_event(self):
        event = Gdk.Event.new(Gdk.EventType.CONFIGURE)
        self.assertEquals(event.type, Gdk.EventType.CONFIGURE)
        self.assertEquals(event.send_event, 0)

        event = Gdk.Event.new(Gdk.EventType.DRAG_MOTION)
        event.x_root, event.y_root = 0, 5
        self.assertEquals(event.x_root, 0)
        self.assertEquals(event.y_root, 5)

class TestGtk(unittest.TestCase):
    def test_uimanager(self):
        self.assertEquals(Gtk.UIManager, overrides.Gtk.UIManager)
        ui = Gtk.UIManager()
        ui.add_ui_from_string(
"""
<ui>
    <menubar name="menubar1"></menubar>
</ui>
"""
)
        menubar = ui.get_widget("/menubar1")
        self.assertEquals(type(menubar), Gtk.MenuBar)

    def test_actiongroup(self):
        self.assertEquals(Gtk.ActionGroup, overrides.Gtk.ActionGroup)
        action_group = Gtk.ActionGroup (name = 'TestActionGroup')
        callback_data = "callback data"

        def test_action_callback_data(action, user_data):
            self.assertEquals(user_data, callback_data);

        def test_radio_action_callback_data(action, current, user_data):
            self.assertEquals(user_data, callback_data);

        action_group.add_actions ([
            ('test-action1', None, 'Test Action 1',
             None, None, test_action_callback_data),
            ('test-action2', Gtk.STOCK_COPY, 'Test Action 2',
              None, None, test_action_callback_data)], callback_data)
        action_group.add_toggle_actions([
            ('test-toggle-action1', None, 'Test Toggle Action 1',
             None, None, test_action_callback_data, False),
            ('test-toggle-action2', Gtk.STOCK_COPY, 'Test Toggle Action 2',
              None, None, test_action_callback_data, True)], callback_data)
        action_group.add_radio_actions([
            ('test-radio-action1', None, 'Test Radio Action 1'),
            ('test-radio-action2', Gtk.STOCK_COPY, 'Test Radio Action 2')], 1,
            test_radio_action_callback_data,
            callback_data)

        expected_results = (('test-action1', Gtk.Action),
                            ('test-action2', Gtk.Action),
                            ('test-toggle-action1', Gtk.ToggleAction),
                            ('test-toggle-action2', Gtk.ToggleAction),
                            ('test-radio-action1', Gtk.RadioAction),
                            ('test-radio-action2', Gtk.RadioAction))

        for action, cmp in zip(action_group.list_actions(), expected_results):
            a = (action.get_name(), type(action))
            self.assertEquals(a,cmp)
            action.activate()

    def test_builder(self):
        self.assertEquals(Gtk.Builder, overrides.Gtk.Builder)

        class SignalTest(GObject.GObject):
            __gtype_name__ = "GIOverrideSignalTest"
            __gsignals__ = {
                "test-signal": (GObject.SIGNAL_RUN_FIRST,
                                GObject.TYPE_NONE,
                                []),
            }


        class SignalCheck:
            def __init__(self):
                self.sentinel = 0

            def on_signal_1(self, *args):
                self.sentinel += 1

            def on_signal_3(self, *args):
                self.sentinel += 3

        signal_checker = SignalCheck()
        builder = Gtk.Builder()

        # add object1 to the builder
        builder.add_from_string(
"""
<interface>
  <object class="GIOverrideSignalTest" id="object1">
      <signal name="test-signal" handler="on_signal_1" />
  </object>
</interface>
""")

        # only add object3 to the builder
        builder.add_objects_from_string(
"""
<interface>
  <object class="GIOverrideSignalTest" id="object2">
      <signal name="test-signal" handler="on_signal_2" />
  </object>
  <object class="GIOverrideSignalTest" id="object3">
      <signal name="test-signal" handler="on_signal_3" />
  </object>
  <object class="GIOverrideSignalTest" id="object4">
      <signal name="test-signal" handler="on_signal_4" />
  </object>
</interface>

""",
            ['object3'])

        # hook up signals
        builder.connect_signals(signal_checker)

        # call their notify signals and check sentinel
        objects = builder.get_objects()
        self.assertEquals(len(objects), 2)
        for obj in objects:
            obj.emit('test-signal')

        self.assertEquals(signal_checker.sentinel, 4)

    def test_dialog(self):
        self.assertEquals(Gtk.Dialog, overrides.Gtk.Dialog)
        dialog = Gtk.Dialog (title='Foo',
                             flags=Gtk.DialogFlags.MODAL,
                             buttons=('test-button1', 1))

        dialog.add_buttons ('test-button2', 2, Gtk.STOCK_CLOSE, Gtk.ResponseType.CLOSE)

        self.assertEquals('Foo', dialog.get_title())
        self.assertTrue(dialog.get_modal())
        button = dialog.get_widget_for_response (1)
        self.assertEquals('test-button1', button.get_label())
        button = dialog.get_widget_for_response (2)
        self.assertEquals('test-button2', button.get_label())
        button = dialog.get_widget_for_response (Gtk.ResponseType.CLOSE)
        self.assertEquals(Gtk.STOCK_CLOSE, button.get_label())

    class TestClass(GObject.GObject):
        __gtype_name__ = "GIOverrideTreeAPITest"

        def __init__(self, tester, int_value, string_value):
            super(TestGtk.TestClass, self).__init__()
            self.tester = tester
            self.int_value = int_value
            self.string_value = string_value

        def check(self, int_value, string_value):
            self.tester.assertEquals(int_value, self.int_value)
            self.tester.assertEquals(string_value, self.string_value)

    def test_tree_store(self):
        self.assertEquals(Gtk.TreeStore, overrides.Gtk.TreeStore)
        self.assertEquals(Gtk.ListStore, overrides.Gtk.ListStore)
        self.assertEquals(Gtk.TreeModel, overrides.Gtk.TreeModel)
        self.assertEquals(Gtk.TreeViewColumn, overrides.Gtk.TreeViewColumn)

        tree_store = Gtk.TreeStore(int, 'gchararray', TestGtk.TestClass)
        parent = None
        for i in range(100):
            label = 'this is child #%d' % i
            testobj = TestGtk.TestClass(self, i, label)
            parent = tree_store.append(parent, (i, label, testobj))

        # len gets the number of children in the root node
        # since we kept appending to the previous node
        # there should only be one child of the root
        self.assertEquals(len(tree_store), 1)

        # walk the tree to see if the values were stored correctly
        parent = None
        i = 0

        (has_children, treeiter) = tree_store.iter_children(parent)
        while (has_children):
           i = tree_store.get_value(treeiter, 0)
           s = tree_store.get_value(treeiter, 1)
           obj = tree_store.get_value(treeiter, 2)
           obj.check(i, s)
           parent = treeiter
           (has_children, treeiter) = tree_store.iter_children(parent)

        self.assertEquals(i, 99)

    def test_list_store(self):
        list_store = Gtk.ListStore(int, str, 'GIOverrideTreeAPITest')
        for i in range(100):
            label = 'this is row #%d' % i
            testobj = TestGtk.TestClass(self, i, label)
            parent = list_store.append((i, label, testobj))

        self.assertEquals(len(list_store), 100)

        # walk the list to see if the values were stored correctly
        i = 0
        (has_more, treeiter) = list_store.get_iter_first()

        while has_more:
            i = list_store.get_value(treeiter, 0)
            s = list_store.get_value(treeiter, 1)
            obj = list_store.get_value(treeiter, 2)
            obj.check(i, s)
            has_more = list_store.iter_next(treeiter)

        self.assertEquals(i, 99)

    def test_tree_view_column(self):
        cell = Gtk.CellRendererText()
        column = Gtk.TreeViewColumn(title='This is just a test',
                                    cell_renderer=cell,
                                    text=0,
                                    style=2)

    def test_text_buffer(self):
        self.assertEquals(Gtk.TextBuffer, overrides.Gtk.TextBuffer)
        buffer = Gtk.TextBuffer()
        tag = buffer.create_tag ('title', font = 'Sans 18')

        self.assertEquals(tag.props.name, 'title')
        self.assertEquals(tag.props.font, 'Sans 18')

        (start, end) = buffer.get_bounds()

        buffer.insert(end, 'HelloHello')
        buffer.insert(end, ' Bob')

        cursor_iter = end.copy()
        cursor_iter.backward_chars(9)
        buffer.place_cursor(cursor_iter)
        buffer.insert_at_cursor(' Jane ')

        (start, end) = buffer.get_bounds()
        text = buffer.get_text(start, end, False)

        self.assertEquals(text, 'Hello Jane Hello Bob')

    def test_buttons(self):
        self.assertEquals(Gtk.Button, overrides.Gtk.Button)

        # test Gtk.Button
        button = Gtk.Button()
        button = Gtk.Button(stock=Gtk.STOCK_CLOSE)
        self.assertEquals(Gtk.STOCK_CLOSE, button.get_label())
        self.assertTrue(button.get_use_stock())
        self.assertTrue(button.get_use_underline())
