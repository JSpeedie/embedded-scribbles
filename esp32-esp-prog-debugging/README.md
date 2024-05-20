# ESP32 Debugging With the ESP-Prog

## Description

What follows is a recounting of the steps I took in order to setup ESP-Prog (or JTAG)
\+ openocd + gdb debugging on my computer.

## 1. Hardware connections

### 1.1 Configuring your ESP-Prog Correctly

The very first step is to make sure your ESP-Prog is configured correctly.
Refer to
[section on the details of the ESP-Prog](https://docs.espressif.com/projects/espressif-esp-iot-solution/en/latest/hw-reference/ESP-Prog_guide.html)
in order to ensure it is. In particular, you want to make sure the two power
jumpers are connected in 3.3V mode if you are debugging the ESP32.

### 1.2 Connecting your ESP-Prog and ESP32

Next, you need to connect your ESP-Prog to the ESP32 you wish to debug. Here is
a schematic for how I connected mine.

<!-- <p align="center"> -->
<!--   <img src="https://raw.githubusercontent.com/wiki/JSpeedie/embedded-scribbles/images/ESP32-Tilting-Ball.png" width="50%"/> -->
<!-- </p> -->

Both the ESP32 and the ESP-Prog should be connected to your development
computer by 2 separate micro-usb to usb cables.

### 1.3 Configuring the Drivers

Even though you are on Linux, you will need to configure the drivers. You can
refer to
[the simple instructions on configuring the drivers](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-guides/jtag-debugging/configure-other-jtag.html#configure-drivers).

Further, if you are using a non-ESP-Prog JTAG debugger, you may want to refer to
[the section on configuring other JTAG interfaces](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-guides/jtag-debugging/configure-other-jtag.html#configure-hardware)
to see what JTAG pin of your debugger should be connected to which GPIO pin of
the ESP32.

&nbsp;

## 2. OpenOCD

### 2.1 Confirming that OpenOCD is installed correctly

The debug process involves running an `openocd` server which `gdb` will connect
to. To confirm that `openocd` is installed correctly, run the following commands:

```bash
cd ~/esp/esp-idf/
get_idf
openocd --version
echo $OPENOCD_SCRIPTS
```

[As explained in the section on setting up OpenOCD](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-guides/jtag-debugging/index.html#setup-of-openocd),
this should confirm that `openocd` is installed on your system.

### 2.2 Configuring OpenOCD for your Target

In order for `openocd` to work correctly, we need to configure it when we launch it.
In our case, debugging an ESP32 with an ESP-Prog, we need to give it two files:
an interface configuration file and a board file:

```bash
openocd -f interface/ftdi/esp32_devkitj_v1.cfg -f target/esp32.cfg
```

For further reading on this part of the process,
[refer to the section on configuring OpenOCD for a specific target](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-guides/jtag-debugging/tips-and-quirks.html#configuration-of-openocd-for-specific-target)

&nbsp;

## 3. GDB

Finally, we need to setup up `gdb` for our purposes.

### 3.1 Create the gdbinit file for our project

As covered [in the section on debugging using the commandline](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-guides/jtag-debugging/using-debugger.html#jtag-debugging-using-debugger-command-line),
in order to avoid running into problems with `gdb`, we will add to the project
we are debugging a `gdbinit` file.

```bash
cd ~/esp/esp-idf/
cd projects/us-test
vim gdbinit
```

We will make the contents of the `gdbinit` file the following:

```
target remote :3333
set remote hardware-watchpoint-limit 2
mon reset halt
maintenance flush register-cache
thb app_main
c
```

### 3.2 Running GDB

To run GDB, we must first ensure our `openocd` server from the earlier is running.
Once it is, we can run the following commands to start `gdb`:

```bash
cd ~/esp/esp-idf/
get_idf
xtensa-esp32-elf-gdb -x gdbinit build/blink.elf
```

&nbsp;

## 4. Conclusion

Once you have completed this process once (in particular, once you have
completed steps 1.1, 1.2, 1.3, and 3.1), then all you need to debug your board
is the following:

### 4.1 A terminal for flashing the board

```bash
cd ~/esp/esp-idf/
get_idf
cd [your project name/path here]
idf.py -p /dev/ttyUSB0 flash monitor
```

### 4.2 A terminal for openocd

```bash
cd ~/esp/esp-idf/
get_idf
openocd -f interface/ftdi/esp32_devkitj_v1.cfg -f target/esp32.cfg
```

### 4.3 A terminal for gdb

```bash
cd ~/esp/esp-idf/
get_idf
cd [your project name/path here]
xtensa-esp32-elf-gdb -x gdbinit build/[name of your project].elf
```



