## Setup

Before you can use the GPIO pins for serial port use, we must first change
the raspberry pi's config.

Run the command:

```bash
sudo raspi-config
```

You will be met with a dialog with several options. You will want to select "3
Interface Options" and from there, "P6 Serial Port", selecting "\<No\>" for
allowing a login shell to be accessible over serial, and "\<Yes\>" for having
the serial port hardware enabled. It will summarize the options you have set,
to which you can hit "Ok", and then you can navigate to "\<Finish\>" and leave
the dialog.

A restart will likely be required.

### Hardware connections

<p align="center">
  <img src="https://raw.githubusercontent.com/wiki/JSpeedie/embedded-scribbles/images/rasp-pi-uart-hardware-connections.jpg" width="50%"/>
</p>

## Running

On the computer the usb side of the usb-to-uart adapter is connected to, run:

```bash
sudo ./uart-chat-client /dev/ttyUSB0
# Or alternatively, we could use a different emulator
sudo screen /dev/ttyUSB0 9600
```

Then, on the raspberry pi, run:

```bash
sudo ./uart-chat-client /dev/serial0
```

The two terminals should now be able to communicate by typing arbitrary data
and hitting enter :)

**Note:** to avoid the use of `sudo` in these commands, we need only add our
user to the `dialout` group:

```bash
sudo usermod -a -G dialout <username>
```
