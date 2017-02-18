#!/usr/bin/python

import gobject
import dbus
import dbus.service
import dbus.mainloop.glib

class Agent(dbus.service.Object):
    @dbus.service.method("net.connman.Agent", in_signature='oa{sv}', out_signature='a{sv}')
    def RequestInput(self, path, fields):
        print(path, fields)
        # TODO: fill the requested fields
        response = None
        return response

def Scan(technology):
    path = "/net/connman/technology/" + technology
    technology = dbus.Interface(bus.get_object("net.connman", path), "net.connman.Technology")
    technology.Scan()

def FindService(ssid):
    for path, properties in manager.GetServices():
        name = properties["Name"]
        #security = extract_list(properties["Security"])
        if (ssid == name):
            return (path, properties)

    return (None, None)

def Connect(ssid, passphrase):
    Scan("wifi")
    path, properties = FindService(ssid)
    if (path is None):
        print("Service with ssid =", ssid, "not found.")
        return

    print("path:", path)
    service = dbus.Interface(bus.get_object("net.connman", path), "net.connmann.Service")
    print("Connecting...");

    try:
        service.Connect(timeout=10000)
        print("Done!")
    except dbus.DBusException as error:
        print("Failed: ", error._dbus_error_name, error.get_dbus_message())

if __name__ == '__main__':
    dbus.mainloop.glib.DBusGMainLoop(set_as_default=True)

    ssid = "Cisco21281"
    passphrase = "A9E544E10F"

    bus = dbus.SystemBus()
    manager = dbus.Interface(bus.get_object('net.connman', "/"), 'net.connman.Manager')
    path = "/test/agent"
    object = Agent(bus, path)
    object.ssid = ssid

    try:
        manager.RegisterAgent(path)
        print("Agent registered!")

    except:
        print("Cannot register connman agent.")

    Connect(ssid, passphrase)

    mainloop = gobject.MainLoop()
    mainloop.run()
