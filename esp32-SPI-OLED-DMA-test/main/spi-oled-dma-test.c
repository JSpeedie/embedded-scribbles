#include <stdio.h>
#include <string.h>
#include <inttypes.h>

#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_heap_caps.h" // For DMA
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

/* Component includes */
#include "esp32-spi-ssd1327.h"


/* Going off  https://learn.adafruit.com/assets/111179 */
#define SPI_MOSI_PIN_NUM 18
/* #define SPI_MISO_PIN_NUM 19 */ // Not used for the OLED
#define SPI_SCK_PIN_NUM  14
#define SPI_CS_PIN_NUM   15
#define DC_PIN_NUM   26
#define RST_PIN_NUM  25

/* SPI0 and SPI1 are reserved so we can choose between SPI2 and SPI3 which
 * are also referred to elsewhere as HSPI and VSPI respectively. Here we
 * commit to using HSPI */
#define SPI_HOST_TAG SPI2_HOST


uint64_t count_total = 0;


void count_task(void *arg) {
    while (1) {
        count_total++;
        vTaskDelay(1);
    }
}


void app_main(void) {
    /* {{{ */
	/* 1. Configure the spi master bus */
    spi_bus_config_t spi_bus_cfg = {
        .miso_io_num = -1,
        .mosi_io_num = SPI_MOSI_PIN_NUM,
        .sclk_io_num = SPI_SCK_PIN_NUM,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
    };

    ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &spi_bus_cfg, SPI_DMA_CH_AUTO));
    /* ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &spi_bus_cfg, SPI_DMA_DISABLED)); */

    /* 2. Configure the spi device */
    spi_device_interface_config_t dev_cfg = {
        .clock_speed_hz = 10 * 1000 * 1000,      // Clock out at 10 MHz
        .mode = 0,                               // SPI mode 0
        .spics_io_num = SPI_CS_PIN_NUM,          // CS pin
        .queue_size = 7,                         // We want to be able to queue 7 transactions at a time
    };
        /* .post_cb = post_transfer_callback,       // We want to call the callback function after a transaction */

    spi_device_handle_t oled_dev_handle;
    ESP_ERROR_CHECK(spi_bus_add_device(SPI_HOST_TAG, &dev_cfg, &oled_dev_handle));

    /* 3. Initialize the remaining GPIO pins */
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << DC_PIN_NUM),
        .mode = GPIO_MODE_OUTPUT,
        .pull_down_en = GPIO_PULLDOWN_ENABLE,
    };
    gpio_config(&io_conf);

    gpio_config_t io_conf2 = {
        .pin_bit_mask = (1ULL << RST_PIN_NUM),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
    };
    gpio_config(&io_conf2);

    /* 4. Create SSD1327 struct for use of the spi_oled functions */
    struct spi_ssd1327 spi_ssd1327 = {
        .dc_pin_num = DC_PIN_NUM,
        .rst_pin_num = RST_PIN_NUM,
        .spi_handle = &oled_dev_handle,
    };
    /* }}} */

    spi_oled_init(&spi_ssd1327);

    printf("screen is on\n");


/* ========================================================================= */
    /* DMA Speed Test */
/* ========================================================================= */

    // Allocate some DMA memory the size of an array representing the whole screen
    uint8_t *oled_screen = heap_caps_malloc(sizeof(uint8_t) * 128 * 64, MALLOC_CAP_DMA);
    /* uint8_t *oled_screen = malloc(sizeof(uint8_t) * 128 * 64); */

    uint8_t whitecircle16[128] = { 0x00, 0x00, 0x0f, 0xff, 0xff, 0xf0, 0x00, 0x00,
                                   0x00, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xf0, 0x00,
                                   0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00,
                                   0x0f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf0,
                                   0x0f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf0,
                                   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                                   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                                   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                                   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                                   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                                   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                                   0x0f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf0,
                                   0x0f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf0,
                                   0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00,
                                   0x00, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xf0, 0x00,
                                   0x00, 0x00, 0x0f, 0xff, 0xff, 0xf0, 0x00, 0x00 };



    // Populate the memory with all black pixels
    for (int i = 0; i < 128; i++) {
        for (int j = 0; j < 64; j++) {
            oled_screen[(64 * i) + j] = 0x00;
        }
    }
    spi_oled_draw_image(&spi_ssd1327, oled_screen);
    vTaskDelay(100 / portTICK_PERIOD_MS);

    /* Start counting task to measure CPU availability */
    TaskHandle_t count_task_task;

    /* xTaskCreatePinnedToCore(count_task, "count_task", 2048, \ */
    /*     (void *)NULL, tskIDLE_PRIORITY, &count_task_task, 0); */
    xTaskCreatePinnedToCore(count_task, "count_task", 2048, \
        (void *)NULL, configMAX_PRIORITIES - 1, &count_task_task, 0);

    for (int a = 0; a < 100; a++) {
        // Draw a circle in the top left 16x16 pixels
        for (int i = 0; i < 16; i++) {
            for (int j = 0; j < 8; j++) {
                oled_screen[(64 * i) + j] = whitecircle16[(8 * i) + j];
            }
        }
        spi_oled_draw_image(&spi_ssd1327, oled_screen);

        // Populate the memory with all black pixels
        for (int i = 0; i < 128; i++) {
            for (int j = 0; j < 64; j++) {
                oled_screen[(64 * i) + j] = 0x00;
            }
        }
        spi_oled_draw_image(&spi_ssd1327, oled_screen);

        // Populate the memory with all white pixels
        for (int i = 0; i < 128; i++) {
            for (int j = 0; j < 64; j++) {
                oled_screen[(64 * i) + j] = 0xff;
            }
        }
        spi_oled_draw_image(&spi_ssd1327, oled_screen);

        // Populate the memory with all black pixels
        for (int i = 0; i < 128; i++) {
            for (int j = 0; j < 64; j++) {
                oled_screen[(64 * i) + j] = 0x00;
            }
        }
        spi_oled_draw_image(&spi_ssd1327, oled_screen);
    }

    printf("count_total = %llu\n", count_total);

    vTaskDelay(10000000 / portTICK_PERIOD_MS);

}
