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

Once you've set it up (as well as made the necessary hardware connections) you need to...

#### 1. Install the Tilting Ball Project

```bash
git clone git@github.com:JSpeedie/embedded-scribbles.git embedded-scribblesGit
cd ~/esp/esp-idf
mkdir projects
```

The tilting ball project is dependent on 2 libraries however, so we need
to install those too before we can compile and run the project.

#### 2. Install the Components Libraries

To install the libraries, follow these 2 series of commands:

```bash
cd
git clone git@github.com:JSpeedie/ESP32-I2C-LSM6DSOX-LIS3MDL-Library.git ESP32-I2C-LSM6DSOX-LIS3MDL-LibraryGit
mkdir ~/esp/esp-idf/projects/esp32-IMU-OLED-tilting-ball/components
cp -r ESP32-I2C-LSM6DSOX-LIS3MDL-LibraryGit/components/esp32-i2c-lsm6dsox-lis3mdl/ ~/esp/esp-idf/esp32-IMU-OLED-tilting-ball/components/esp32-i2c-lsm6dsox-lis3mdl/
```

```bash
cd
git clone git@github.com:JSpeedie/ESP32-SPI-SSD1327-Library.git ESP32-SPI-SSD1327-LibraryGit
mkdir ~/esp/esp-idf/projects/esp32-IMU-OLED-tilting-ball/components
cp -r ESP32-SPI-SSD1327-LibraryGit/components/esp32-spi-ssd1327/ ~/esp/esp-idf/esp32-IMU-OLED-tilting-ball/components/esp32-spi-ssd1327/
```

Tada! You should have both libraries installed for your project now.

#### 3. Compiling and Running

After installing the project and the libraries it depends on (and making the
hardware connections. See below), we are finally ready to compile and run the
project:

```bash
cd ~/esp/esp-idf
get_idf
cd projects/tilting-ball
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
