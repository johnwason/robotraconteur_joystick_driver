# Robot Raconteur Joystick Driver

This package provides a joystick and gamepad Robot Raconteur driver service. It implements the `com.robotraconteur.hid.joystick.Joystick` standard type. Rumble and constant-force-feedback are implemented for devices that support haptic feedback.

This driver is based on the [SDL2 Library](https://www.libsdl.org/) joystick, gamepad, and haptic features.

By default the update rate is 100 Hz.

## Usage

### List available devices

List in human readable table:

    robotraconteur_joystick_driver --list

List in YAML format to stdout:

    robotraconteur_joystick_driver --list-yaml

List in YAML format and save to file:

    robotraconteur_joystick_driver --list-yaml-save=<filename>

### Identify a device

Identify a device. Hold a button on the joystick/gamepad to determine its ID:

    robotraconteur_joystick_driver --identify

### Run the joystick service

Run the joystick service for the specified device:

    robotraconteur_joystick_driver --joystick_id=<id>

### Robot Raconteur command line options

All Robot Raconteur node setup command line options are supported. See [Command Line Options](https://github.com/robotraconteur/robotraconteur/wiki/Command-Line-Options)

By default the node listens on TCP port 64234 and have the NodeName com.robotraconteur.hid.joystickN, where N is the ID of the joystick. These can be overridden using Robot Raconteur node setup command line options. For example,

    robotraconteur_joystick_driver --joystick_id=1 --robotraconteur-tcp-port=54444 --robotraconteur-node-name=joystick

will use Joystick 1, will listen on TCP port 54444, and will have NodeName "joystick".

Note that it is necessary to change the TCP port if multiple joysticks services are running.

### Client Connection

For the default options on localhost, the following connection URL can be used:

    rr+tcp://localhost:64234?service=joystick

### Example Client

A simple Python example that reads the gamepad and rumbles periodically:

    from RobotRaconteur.Client import *
    import time

    c = RRN.ConnectService("rr+tcp://localhost:64234?service=joystick")

    rumble_counter = 1000

    while True:

        pad_state, _ = c.gamepad_state.PeekInValue()
        print(str(pad_state.left_x) + "," + str(pad_state.left_y) + ","
            + str(pad_state.trigger_left) + "," + str(pad_state.trigger_right) + ","
            + hex(pad_state.buttons))

        rumble_counter += 1
        if (rumble_counter > 50):
            rumble_counter = 0
            c.rumble(1.0,2.0)

        time.sleep(0.1)

The joystick driver provides both "Joystick" and "Gamepad" style SDL2 interfaces. The "Gamepad" interface is guaranteed to provide a consistent XBox gamepad button and stick style mapping. The "Joystick" passes the manufacturer mapping of the device, and has few constraints an the configuration of the "axes", "buttons", and "hats" that are read by the SDL2 driver. See the [SDL2 Documentation](https://wiki.libsdl.org/APIByCategory) for more information on how to interpret the "Joystick" and "Gamepad" readings.

## License

Apache 2.0 Licensed by Wason Technology, LLC

Author: John Wason, PhD
