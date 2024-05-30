set(srcs "spi-oled-dma-test.c")

idf_component_register(SRCS "${srcs}"
                    REQUIRES driver
                    PRIV_REQUIRES esp32-spi-ssd1327
                    INCLUDE_DIRS ".")
