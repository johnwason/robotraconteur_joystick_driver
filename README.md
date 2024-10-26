<p align="center"><img src="https://raw.githubusercontent.com/robotraconteur/robotraconteur/refs/heads/master/docs/figures/logo-header.svg"></p>

# Robot Raconteur Joystick Driver

This package provides a joystick and gamepad Robot Raconteur driver service. It implements the `com.robotraconteur.hid.joystick.Joystick` standard type. Rumble and constant-force-feedback are implemented for devices that support haptic feedback.

This driver is based on the [SDL2 Library](https://www.libsdl.org/) joystick, gamepad, and haptic features.

By default the update rate is 100 Hz.

Binaries for Windows are available on the [Releases](https://github.com/robotraconteur-contrib/robotraconteur_joystick_driver/releases)
page. Use Docker for Linux.

## Connection Info

The default connection information is as follows. These details may be changed using `--robotraconteur-*` command
line options when starting the service. Also see the
[Robot Raconteur Service Browser](https://github.com/robotraconteur/RobotRaconteur_ServiceBrowser) to detect
services on the network.

- URL: `rr+tcp://localhost:64234?service=joystick`
- Device Name: `joy0` or device name in the configuration file
- Node Name: `com.robotraconteur.hid.joystickN` where N is the ID of the joystick
- Service Name: `joystick`
- Root Object Type:
  - `com.robotraconteur.hid.joystick.Joystick`

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

### Command Line Options

The following command line arguments are available:

* `--joystick-info-file=` - The joystick info file. Info files are available in the `config/` directory. See [robot info file documentation](https://github.com/robotraconteur/robotraconteur_standard_robdef/blob/master/docs/info_files/joystick.md)
* `--joystick-id=` - The ID of the joystick to use. The default is 0. The ID is an integer that is used to identify the joystick. Use the `--list` option to list the available joysticks and their IDs.
* `--list` - List the available joysticks and their IDs.
* `--list-yaml` - List the available joysticks and their IDs in YAML format.
* `--list-yaml-save=` - List the available joysticks and their IDs in YAML format and save to a file.
* `--identify` - Identify the joystick. Hold a button on the joystick/gamepad to determine its ID.

The [common Robot Raconteur node options](https://github.com/robotraconteur/robotraconteur/wiki/Command-Line-Options) are also available.

If more than one joystick service is running, the TCP port must be changed using the
`--robotraconteur-tcp-port=` command line option.

### Robot Raconteur command line options

All Robot Raconteur node setup command line options are supported. See [Robot Raconteur Node Command Line Options](https://github.com/robotraconteur/robotraconteur/wiki/Command-Line-Options)

By default the node listens on TCP port 64234 and have the NodeName com.robotraconteur.hid.joystickN, where N is the ID of the joystick. These can be overridden using Robot Raconteur node setup command line options. For example,

    robotraconteur_joystick_driver --joystick_id=1 --robotraconteur-tcp-port=54444 --robotraconteur-node-name=joystick

will use Joystick 1, will listen on TCP port 54444, and will have NodeName "joystick".

Note that it is necessary to change the TCP port if multiple joysticks services are running.

## Example Client

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

## Docker Usage

```
sudo docker run --rm --net=host --privileged -v /var/run/robotraconteur:/var/run/robotraconteur -v /var/lib/robotraconteur:/var/lib/robotraconteur -e JOYSTICK_INFO_FILE=/config/joy0_default_config.yml -e JOYSTICK_ID=0 wasontech/robotraconteur-joystick-driver
```

It may be necessary to mount a docker "volume" to access configuration yml files that are not included in the docker image.
See the docker documentation for instructions on mounting a local directory as a volume so it can be accessed inside the docker.

To list the available devices:

```
sudo docker run --rm --net=host --privileged -v /var/run/robotraconteur:/var/run/robotraconteur -v /var/lib/robotraconteur:/var/lib/robotraconteur  wasontech/robotraconteur-joystick-driver /usr/local/bin/robotraconteur_joystick_driver --list
```

## License

Apache 2.0 Licensed by Wason Technology, LLC

Author: John Wason, PhD

## Acknowledgment

This work was supported in part by the Advanced Robotics for Manufacturing ("ARM") Institute under Agreement Number W911NF-17-3-0004 sponsored by the Office of the Secretary of Defense. The views and conclusions contained in this document are those of the authors and should not be interpreted as representing the official policies, either expressed or implied, of either ARM or the Office of the Secretary of Defense of the U.S. Government. The U.S. Government is authorized to reproduce and distribute reprints for Government purposes, notwithstanding any copyright notation herein.

This work was supported in part by the New York State Empire State Development Division of Science, Technology and Innovation (NYSTAR) under contract C160142.

![](https://github.com/robotraconteur/robotraconteur/blob/master/docs/figures/arm_logo.jpg?raw=true)
![](https://github.com/robotraconteur/robotraconteur/blob/master/docs/figures/nys_logo.jpg?raw=true)
