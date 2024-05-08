set(srcs "tilting-ball.c")

idf_component_register(SRCS "${srcs}"
                    REQUIRES driver
                    PRIV_REQUIRES esp32-i2c-lsm6dsox-lis3mdl
                    PRIV_REQUIRES esp32-spi-ssd1327
                    INCLUDE_DIRS ".")
