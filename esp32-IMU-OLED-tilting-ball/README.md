# esp32-IMU-OLED-tilting-ball

## Description

This is a simple project that makes use of a 128 by 128 OLED display and a
9 Degrees-of-Freedom IMU to simulate a ball rolling on a platform. The project
uses FreeRTOS (in particular Symmetric Multiprocessing/Multitasking), semaphores
(to achieve synchronization between tasks), esp-idf, SPI, and I2C.

## Setup

If you wish to run the program, below are the steps:

### Software setup

I used the ESP IDF for development of this project. In order to recreate
this example project, you will have to install the ESP IDF as explained at
[https://docs.espressif.com/projects/esp-idf/en/stable/esp32/get-started/linux-macos-setup.html](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/get-started/linux-macos-setup.html)

Once you've set it up, all you need to do for development (after making the
necessary hardware connections, of course) is:

```bash
git clone git@github.com:JSpeedie/embedded-scribbles.git embedded-scribblesGit
cd ~/esp/esp-idf
get_idf
mkdir projects
cd projects
cp -r ~/embedded-scribblesGit/esp32-IMU-OLED-tilting-ball projects/tilting-ball
cd tilting-ball
idf.py -p /dev/ttyUSB0 flash monitor
```

### Hardware connections

Below is the schematic I used for the example program.

<p align="center">
  <img src="https://raw.githubusercontent.com/wiki/JSpeedie/embedded-scribbles/images/ESP32-Tilting-Ball.png" width="50%"/>
</p>

Of course you will also need to connect a micro usb to usb cable between the
ESP32 and your development machine in order to flash the program to the ESP32
and to give it power.
