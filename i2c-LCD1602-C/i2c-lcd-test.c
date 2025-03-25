#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <unistd.h>


int main(int argc, char **argv) {
	if (argc != 3) {
		printf("Invalid Number of Arguments...\n");
		printf("Usage: ./i2c-lcd-test <path-to-i2c-bus> <i2c-peripheral-address\n");
		return -1;
	}

	/* Parse the i2c lcd device path from the commandline args */
	char i2c_bus_path[512];
	strncpy(i2c_bus_path, argv[1], sizeof(i2c_bus_path));
	/* Parse the i2c peripheral address from the commandline args */
	int i2c_peripheral_addr;
	sscanf(argv[2], "%d", &i2c_peripheral_addr);

	int i2c_lcd_fd;
	uint8_t data[2];

	/* If opening the i2c lcd device failed */
	if ( (i2c_lcd_fd = open(i2c_bus_path, O_RDWR)) < 0) {
		fprintf(stderr, "Failed to open the i2c lcd device\n");
		return -1;
	}

	/* Set the peripheral address for the controller */
	if (-1 == ioctl(i2c_lcd_fd, I2C_SLAVE, i2c_peripheral_addr)) {
		fprintf(stderr, "Failed to set the peripheral address for the i2c controller\n");
		return -1;
	}


	/* At this point, we should be ready to write to the i2c lcd */



	int user_input_len;
	char user_input[512];

	while (1) {
		/* Read a message from stdin */
		if (-1 == (user_input_len = read(STDIN_FILENO, user_input, sizeof(user_input)))) {
			fprintf(stderr, "Failed to read user input!\n");
		} else {
			/* Send something to the i2c device to get it to write the text */
		}
	}

	close(i2c_lcd_fd);

	return 0;
}
