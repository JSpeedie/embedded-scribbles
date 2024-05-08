#include <math.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_system.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "driver/i2c_master.h"

#include "tilting-ball.h"

/* Component includes */
#include "esp32-spi-ssd1327.h"
#include "esp32-i2c-lsm6dsox.h"
#include "esp32-i2c-lis3mdl.h"


/* SPI Defines {{{ */
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
/* }}} */

/* I2C Defines {{{ */
#define I2C_BUS_PORT 0
/* Going off  https://learn.adafruit.com/assets/111179 */
#define I2C_SDA_PIN_NUM 23
#define I2C_SCL_PIN_NUM 22
/* }}} */


SemaphoreHandle_t dof_data_semaphore = NULL;
i2c_master_dev_handle_t *magnetometer_handle;
i2c_master_dev_handle_t *accelgyro_handle;
struct i2c_lsm6dsox *i2c_lsm6dsox;
struct i2c_lis3mdl *i2c_lis3mdl;
struct dof_data dof_data;
struct game_state game_state = {
    .ball_p_x = 128.0/2,
    .ball_p_y = 128.0/2,
    .ball_v_x = 0.0,
    .ball_v_y = 0.0,
    .ball_a_x = 0.0,
    .ball_a_y = 0.0,
    .board_pitch = 0.0,
    .board_roll = 0.0,
};
double tick_period_s;
/* Assuming the screen is square, as ours is (128x128), take the screen
 * diagonal (3.8cm), divide that by 2 (1.9cm), take the square root (1.378cm,
 * the height and width of the screen), divide that by the number of pixels
 * along the width (128) (0.010768cm), and convert to metres (0.00010768m) */
/* double pixel_width = 0.00010768; */
double pixel_width = 0.10768; // TODO: inaccurate, but used as a scaled down 1000x version
double ball_mass = 0.25; // In kg
double ball_radius;
double gravity_constant = 9.81;
double board_friction = 0.5;


void paint_oled(void *arg) {
    /* SPI OLED Initialization {{{ */
	/* 1. Configure the spi master bus */
    spi_bus_config_t spi_bus_cfg = {
        .miso_io_num = -1,
        .mosi_io_num = SPI_MOSI_PIN_NUM,
        .sclk_io_num = SPI_SCK_PIN_NUM,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
    };

    ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &spi_bus_cfg, SPI_DMA_CH_AUTO));

    /* 2. Configure the spi device */
    spi_device_interface_config_t dev_cfg = {
        .clock_speed_hz = 10 * 1000 * 1000,      // Clock out at 10 MHz
        .mode = 0,                               // SPI mode 0
        .spics_io_num = SPI_CS_PIN_NUM,          // CS pin
        .queue_size = 7,                         // We want to be able to queue 7 transactions at a time
    };

    spi_device_handle_t *oled_dev_handle = malloc(sizeof(spi_device_handle_t));
    ESP_ERROR_CHECK(spi_bus_add_device(SPI_HOST_TAG, &dev_cfg, oled_dev_handle));

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
    struct spi_ssd1327 *spi_ssd1327 = malloc(sizeof(struct spi_ssd1327));
    spi_ssd1327->dc_pin_num = DC_PIN_NUM;
    spi_ssd1327->rst_pin_num = RST_PIN_NUM;
    spi_ssd1327->spi_handle = oled_dev_handle;

    spi_oled_init(spi_ssd1327);
    /* }}} */

    /* Paint the whole screen black */
    spi_oled_draw_square(spi_ssd1327, 0, 0, 128, 128, SSD1327_GS_0);
    printf("Black 128x128 square painted\n");

    /* Set the frequency of the loop in this function to 3 ticks */
    const TickType_t taskFrequency = 3;
    TickType_t lastWakeTime;
    double ball_p_x = game_state.ball_p_x;
    double ball_p_y = game_state.ball_p_y;

    printf("About to start painting loop\n");

    while (1) {
        /* Update the lastWakeTime variable to have the current time */
        lastWakeTime = xTaskGetTickCount();

        /* Unpaint the circle at its previous location */
        spi_oled_draw_square(spi_ssd1327, (uint8_t) ball_p_x, (uint8_t) ball_p_y, \
            16, 16, SSD1327_GS_0);

        /* Update the stored location of the ball */
        if (xSemaphoreTake(dof_data_semaphore, portMAX_DELAY) == pdTRUE) {
            ball_p_x = game_state.ball_p_x;
            ball_p_y = game_state.ball_p_y;
            xSemaphoreGive(dof_data_semaphore);
        }

        /* Paint the circle at its new location */
        spi_oled_draw_circle(spi_ssd1327, (uint8_t) ball_p_x, (uint8_t) ball_p_y);

        /* Delay such that this loop executes every 'taskFrequency' ticks */
        vTaskDelayUntil(&lastWakeTime, taskFrequency);
    }
}


/** Take a struct containing both pointers to where the 9 DOF sensor data
 * is stored (so it can update it) and game state data so it can adjust
 * it as well */
void get_9dof_data(void *arg) {
    /* Set the frequency of the loop in this function to 2 ticks */
    const TickType_t taskFrequency = 3;
    TickType_t lastWakeTime;

	/* I2C 9 DOF Initialization {{{ */
    /* 1. Configure the i2c master bus */
	i2c_master_bus_config_t i2c_mst_config = {
		.clk_source = I2C_CLK_SRC_DEFAULT,
		.i2c_port = I2C_BUS_PORT,
		.scl_io_num = I2C_SCL_PIN_NUM,
		.sda_io_num = I2C_SDA_PIN_NUM,
		.glitch_ignore_cnt = 7,
		.flags.enable_internal_pullup = false
	};

	i2c_master_bus_handle_t bus_handle;
	ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_mst_config, &bus_handle));

	/* 2. Configure the LIS3MDL (magnetometer) */
	i2c_device_config_t magnetometer_cfg = {
		.dev_addr_length = I2C_ADDR_BIT_LEN_7,
		.device_address = 0x1E,
		.scl_speed_hz = 100000,
	};

	magnetometer_handle = malloc(sizeof(i2c_master_dev_handle_t));
	ESP_ERROR_CHECK(i2c_master_bus_add_device(bus_handle, &magnetometer_cfg, \
        magnetometer_handle));

	/* 3. Configure the LSM6DSOX (accelerometer + gyroscope) */
	i2c_device_config_t accelgyro_cfg = {
		.dev_addr_length = I2C_ADDR_BIT_LEN_7,
		.device_address = 0x6A,
		.scl_speed_hz = 100000,
	};

	accelgyro_handle = malloc(sizeof(i2c_master_dev_handle_t));
	ESP_ERROR_CHECK(i2c_master_bus_add_device(bus_handle, &accelgyro_cfg, \
        accelgyro_handle));
    printf("about to initialize i2c devs\n");

    /* 4a. Turn on and set operation control for accelerometer and gyro */
    i2c_lsm6dsox = malloc(sizeof(struct i2c_lsm6dsox));
    i2c_lsm6dsox->i2c_handle = accelgyro_handle;
    esp_i2c_lsm6dsox_begin(i2c_lsm6dsox);
    printf("I2C lsm6dsox initialized\n");
    /* 4b. Turn on and set operation control for magnetometer */
    i2c_lis3mdl = malloc(sizeof(struct i2c_lis3mdl));
    i2c_lis3mdl->i2c_handle = magnetometer_handle;
    /* esp_i2c_lis3mdl_begin(i2c_lis3mdl); */
    printf("I2C lis3mdl initialized\n");
    /* }}} */

    printf("About to start data loop\n");

    while (1) {
        /* Update the lastWakeTime variable to have the current time */
        lastWakeTime = xTaskGetTickCount();

        /* esp_i2c_lsm6dsox_get_gyro_data(i2c_lsm6dsox, dof_data.g_xyz); */
        /* /1* float g_x = esp_i2c_lsm6dsox_get_gyro_x(i2c_lsm6dsox); *1/ */
        /* /1* float g_y = esp_i2c_lsm6dsox_get_gyro_y(i2c_lsm6dsox); *1/ */
        /* /1* float g_z = esp_i2c_lsm6dsox_get_gyro_z(i2c_lsm6dsox); *1/ */
        /* /1* Convert from mdps (millidegrees per second) to dps (degrees per second) *1/ */
        /* dof_data.g_xyz[0] /= 1000; */
        /* dof_data.g_xyz[1] /= 1000; */
        /* dof_data.g_xyz[2] /= 1000; */

        esp_i2c_lsm6dsox_get_accel_data(i2c_lsm6dsox, dof_data.a_xyz);
        /* Convert from mg (milligravity, not milligrams) to g (gravity)
            * -> /= 1000
            * Convert from g to m/s^2 (on earth at sea leavel)
            * -> *= 9.81
            * Combining both operations
            * -> /= (1000 / 9.81) -> /= 101.94 */
        game_state.ball_a_x = dof_data.a_xyz[0] / 101.94;
        game_state.ball_a_y = dof_data.a_xyz[1] / 101.94;
        /* game_state.ball_a_z = dof_data.a_xyz[2] / 101.94; */
        double acc_z = dof_data.a_xyz[2] / 101.94;

        /* esp_i2c_lis3mdl_get_data(i2c_lis3mdl, t->dof_data.m_xyz); */

        /* printf("gyro: (% #7.2fdps % #7.2fdps % #7.2fdps)    " \ */
        /*     "accel: (% #7.3fm/s^2 % #7.3fm/s^2 % #7.3fm/s^2)\n", \ */
        /*     dof_data.g_xyz[0], dof_data.g_xyz[1], dof_data.g_xyz[2], \ */
        /*     game_state.ball_a_x, game_state.ball_a_y, acc_z); */




        /* /1* MY OWN ATTEMPT: Update the location of the ball based on the accelerometer data. */
        /*  * '-=' because of the orientation of the OLED on my */
        /*  * breadboard relative to the orientation of the IMU *1/ */
        /* game_state.ball_v_x -= (dof_data.a_xyz[0] * tick_period_s * ((uint16_t) taskFrequency)); */
        /* game_state.ball_v_y += (dof_data.a_xyz[1] * tick_period_s * ((uint16_t) taskFrequency)); */
        /* game_state.ball_p_x += game_state.ball_v_x / pixel_width; */
        /* game_state.ball_p_y += game_state.ball_v_y / pixel_width; */


        /* FORCE: Update the location of the ball. */

        /* For my scenario I decided that:
            * pitch = rotation around the x axis (will affect the y coord)
            * roll = rotation around the y axis (will affect the x coord)
            */

        // TODO: problem?: these calculations don't generate NaN's for the
        // pitch and roll, but they don't differentiate between 85 deg and
        // what would be 95 deg - they both just end up as 85
        game_state.board_pitch = \
            atan(game_state.ball_a_y / sqrt(game_state.ball_a_x * game_state.ball_a_x + acc_z * acc_z));
        game_state.board_roll = \
            atan(game_state.ball_a_x / sqrt(game_state.ball_a_y * game_state.ball_a_y + acc_z * acc_z));
		// TODO: there's a problem with these derivations: one of them gives
		// NaN when the IMU is close to 90 degrees rotated around one axis
        /* game_state.board_pitch = atan2(game_state.ball_a_y, acc_z); */
        /* game_state.board_roll = asin(game_state.ball_a_x / gravity_constant); */

        /* printf("pitch: (% #3.2lf°)    roll: (% #3.2lf°)\n", \ */
        /*     game_state.board_pitch * 57.2958, game_state.board_roll * 57.2958); */

        double force_normal_x = \
            ball_mass * gravity_constant * cos(game_state.board_roll);
        /* Aka force_downhill_x / ball_mass = ... */
        game_state.ball_a_x += \
            gravity_constant * sin(game_state.board_roll);

        double force_normal_y = \
            ball_mass * gravity_constant * cos(game_state.board_pitch);
        /* Aka force_downhill_y / ball_mass = ... */
        game_state.ball_a_y += \
            gravity_constant * sin(game_state.board_pitch);

        /* '-=' because of the orientation of the OLED on my
            * breadboard relative to the orientation of the IMU */
        game_state.ball_v_x -= (game_state.ball_a_x * tick_period_s * ((uint16_t) taskFrequency));
        game_state.ball_v_y += (game_state.ball_a_y * tick_period_s * ((uint16_t) taskFrequency));

        if (xSemaphoreTake(dof_data_semaphore, portMAX_DELAY) == pdTRUE) {
            game_state.ball_p_x += game_state.ball_v_x / pixel_width;
            game_state.ball_p_y += game_state.ball_v_y / pixel_width;

            /* Make sure the ball's position and velocity respect the bounds */
            if (game_state.ball_p_x < 0) {
                game_state.ball_p_x = 0;
                game_state.ball_v_x = 0;
                game_state.ball_a_x = 0;
            } else if (game_state.ball_p_x > 127 - 16) {
                game_state.ball_p_x = 127 - 16;
                game_state.ball_v_x = 0;
                game_state.ball_a_x = 0;
            }
            if (game_state.ball_p_y < 0) {
                game_state.ball_p_y = 0;
                game_state.ball_v_y = 0;
                game_state.ball_a_y = 0;
            } else if (game_state.ball_p_y > 127 - 16) {
                game_state.ball_p_y = 127 - 16;
                game_state.ball_v_y = 0;
                game_state.ball_a_y = 0;
            }

            /* printf("ball_p: (% #3.2lfpx % #3.2lfpx)    " \ */
            /*        "ball_v: (% #3.2lfm/s % #3.2lfm/s)    " \ */
            /*        "ball_a: (% #3.2lfm/s^2 % #3.2lfm/s^2)\n", \ */
            /*     game_state.ball_p_x, game_state.ball_p_y, */
            /*     game_state.ball_v_x, game_state.ball_v_y, */
            /*     game_state.ball_a_x, game_state.ball_a_y); */
            /* printf("ball_a: (% #3.2lfm/s/tick % #3.2lfm/s/tick)\n", \ */
            /*     game_state.ball_a_x * tick_period_s * ((uint16_t) taskFrequency), \ */
            /*     game_state.ball_a_y * tick_period_s * ((uint16_t) taskFrequency)); */

            xSemaphoreGive(dof_data_semaphore);
        }

        /* Delay such that this loop executes every 'taskFrequency' ticks */
        vTaskDelayUntil(&lastWakeTime, taskFrequency);
    }
}


void app_main(void) {
    /* Create the variables that will be shared between
     * the paint_oled and the 9dof_data tasks */

    /* This is used to adjust physics calculations performed using m/s or m/s^2
     * for our data which will be tied to the tick rate of the ESP32 and
     * the frequency of the task in ticks */
    tick_period_s = portTICK_PERIOD_MS / 1000.0;
    ball_radius = pixel_width * 16 / 2;

    dof_data.g_xyz = malloc(sizeof(float) * 3);
    dof_data.g_xyz[0] = 0.0f;
    dof_data.g_xyz[1] = 0.0f;
    dof_data.g_xyz[2] = 0.0f;

    dof_data.a_xyz = malloc(sizeof(float) * 3);
    dof_data.a_xyz[0] = 0.0f;
    dof_data.a_xyz[1] = 0.0f;
    dof_data.a_xyz[2] = 0.0f;

    dof_data.m_xyz = malloc(sizeof(float) * 3);
    dof_data.m_xyz[0] = 0.0f;
    dof_data.m_xyz[1] = 0.0f;
    dof_data.m_xyz[2] = 0.0f;

    vSemaphoreCreateBinary(dof_data_semaphore);
    if (dof_data_semaphore == NULL) {
        printf("ERROR: creating semaphore!\n");
    }

    TaskHandle_t paint_oled_task;
    TaskHandle_t get_9dof_data_task;

    xTaskCreatePinnedToCore(paint_oled, "paint_oled", 20480, \
        (void *)NULL, 10, &paint_oled_task, 0);
    xTaskCreatePinnedToCore(get_9dof_data, "get_9dof_data", 20480, \
        (void *)NULL, 10, &get_9dof_data_task, 1);
}
