# esp32-SPI-OLED-DMA-test

## Description

This is a simple project that demonstrates the efficiency differences between
using DMA for SPI communication with a display and using SPI communication with
a display without DMA.


## Results

In my testing, the DMA would fairly consistently print a count of `628` and the
non-DMA code would fairly consistently print a count of `599`. It's clear that
the code is not much faster when using DMA, but it was *always* faster.
Furthermore, in testing I found that I can send ~32 times(!) more data per
`spi_device_transmit()` call if DMA was enabled. While this doesn't demonstrate
how using DMA can save the CPU from wasting cycles doing basic work, it does
show that the ESP32 has perhaps been set up to encourage the use of DMA as SPI
transactions that use DMA have much bigger maximum transaction sizes.


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
cp -r ~/embedded-scribblesGit/esp32-SPI-OLED-DMA-test/ projects/spi-oled-dma-test
```

#### 2. Compiling and Running

After installing the project and the libraries it depends on (and making the
hardware connections. See below), we are finally ready to compile and run the
project:

```bash
cd ~/esp/esp-idf
get_idf
cd projects/spi-oled-dma-test
idf.py -p /dev/ttyUSB0 flash monitor
```

#### 3. Comparing DMA SPI and non-DMA SPI

There are 4 lines that must be changed to switch between running the program
using DMA and running it without. When you clone the repo, by default you will
find two lines like this in `main/spi-oled-dma-test.c`:

```C
    ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &spi_bus_cfg, SPI_DMA_CH_AUTO));
    /* ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &spi_bus_cfg, SPI_DMA_DISABLED)); */
```

The first line tells the ESP32 to use DMA for this SPI bus. To run the program
using DMA, the first of these two lines must be uncommented, and the second
(which tells the ESP32 to *disable* DMA for the SPI bus) must be commented out.
To run the test without DMA, comment out the first line and uncomment the
second line. This is not all that you need to do to switch between DMA and
non-DMA. You will also find another pair of lines further down in the file:

```C
    uint8_t *oled_screen = heap_caps_malloc(sizeof(uint8_t) * 128 * 64, MALLOC_CAP_DMA);
    /* uint8_t *oled_screen = malloc(sizeof(uint8_t) * 128 * 64); */
```

The first line allocates memory that the program uses for an array which it sends
to the display, but it does so in a particular way that makes the memory suitable
for use with DMA. If you are running the program with DMA, make sure the
`heap_caps_malloc()` line is uncommented and the second line is commented. If you
are running the test without DMA, make sure the first of these two lines
is commented out and the second one is uncommented.

Swapping which lines are commented out in both of these pairs is all you need
to do to switch between running the program with DMA and without DMA.

### Hardware connections

Below is the schematic I used for the example program. It is the same as
the wiring setup for the tilting ball game.

<p align="center">
  <img src="https://raw.githubusercontent.com/wiki/JSpeedie/embedded-scribbles/images/ESP32-Tilting-Ball.png" width="50%"/>
</p>

Of course you will also need to connect a micro usb to usb cable between the
ESP32 and your development machine in order to flash the program to the ESP32
and to give it power.

