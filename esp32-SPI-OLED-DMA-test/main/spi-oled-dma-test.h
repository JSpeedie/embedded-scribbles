#include <inttypes.h>

#include "driver/spi_master.h"


void spi_oled_init(spi_device_handle_t *oled_dev_handle);

void spi_oled_reset(void);

void spi_oled_send_cmd(spi_device_handle_t *oled_dev_handle, uint8_t cmd);

void spi_oled_send_data(spi_device_handle_t *oled_dev_handle, void * data, uint32_t data_len_bits);

void spi_oled_draw_circle(spi_device_handle_t *oled_dev_handle, uint8_t x, uint8_t y);

void spi_oled_draw_square(spi_device_handle_t *oled_dev_handle, uint8_t x, uint8_t y, uint8_t width, uint8_t height, uint8_t whiteblack);
