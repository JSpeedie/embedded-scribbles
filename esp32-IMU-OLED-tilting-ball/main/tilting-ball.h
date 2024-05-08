#ifndef __TILTING_BALL_
#define __TILTING_BALL_

#include <inttypes.h>

#include "freertos/semphr.h"

#include "esp32-spi-ssd1327.h"
#include "esp32-i2c-lsm6dsox.h"
#include "esp32-i2c-lis3mdl.h"


struct dof_data {
	float *g_xyz;
	float *a_xyz;
	float *m_xyz;
};

struct game_state {
	double ball_p_x; /* These are stored in pixels (px) and range from 0 <= x <= 127 - 16 */
	double ball_p_y;
	double ball_v_x; /* These are stored in (m/s) */
	double ball_v_y;
	double ball_a_x; /* These are stored in (m/s^2) */
	double ball_a_y;
	double board_pitch; /* These are stored in Radians */
	double board_roll;
};


#endif
