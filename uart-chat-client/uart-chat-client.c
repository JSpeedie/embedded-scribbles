#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/select.h>
#include <termios.h>
#include <unistd.h>


int main(int argc, char **argv) {
	if (argc != 2) {
		printf("Invalid Number of Arguments...\n");
		printf("Usage: ./uart-chat-client <path-to-serial-device>\n");
		return -1;
	}

	/* Parse the serial device path from the commandline args */
	char serial_dev_path[512];
	strncpy(serial_dev_path, argv[1], sizeof(serial_dev_path));

	int serial_fd;
	int send_msg_len;
	char send_msg[256];
	int recv_msg_len;
	char recv_msg[256];
	/* Serial port options */
	struct termios options;

	/* If opening the serial port failed */
	if ( (serial_fd = open(serial_dev_path, O_RDWR | O_NDELAY | O_NOCTTY | O_SYNC)) < 0) {
		fprintf(stderr, "Failed to open the serial port\n");
		return -1;
	}

	/* Set the settings for the serial port */
	if (0 != tcgetattr(serial_fd, &options)) {
		fprintf(stderr, "Failed to get term attributes\n");
		return -1;
	}

	/* Set control mode flags */
	options.c_cflag |= CS8; // Set number of data bytes to 8
	options.c_cflag |= CLOCAL; // Disable modem-specific signals
	options.c_cflag |= CREAD; // Allow reading

	/* Set input flags */
	options.c_iflag |= IGNPAR; // Ignore parity errors

	/* Set output flags */
	options.c_oflag = 0; // Don't set output flags

	/* Set local flags */
	options.c_lflag = 0; // Don't set local flags

	// 1 byte at a time, no timer
	options.c_cc[VMIN] = 1;
	options.c_cc[VTIME] = 0;

	/* Set input/output baud rates */
	cfsetispeed(&options, B9600);
	cfsetospeed(&options, B9600);

	/* Apply the settings for the serial port */
	tcflush(serial_fd, TCIOFLUSH);
	if (0 != tcsetattr(serial_fd, TCSANOW, &options)) {
		fprintf(stderr, "Failed to set term attributes\n");
		return -1;
	}

	/* select() variables */
	fd_set wset;
	fd_set rset;
	int maxfd;

	/* select() initialization */
	FD_ZERO(&wset);
	FD_SET(serial_fd, &wset);
	FD_ZERO(&rset);
	FD_SET(serial_fd, &rset);
	FD_SET(STDIN_FILENO, &rset);
	fd_set wset_stable = wset; // Make a backup of each set since
	fd_set rset_stable = rset; // select() is destructive
	maxfd = serial_fd + 1;

	while (1) {
		/* Recreate the FD sets */
		wset = wset_stable;
		rset = rset_stable;

		select(maxfd, &rset, &wset, NULL, NULL);

		/* If there is data to be read from the serial device */
		if (FD_ISSET(serial_fd, &rset)) {
			/* Read a message on the serial port through the GPIO */
			if (-1 == (recv_msg_len = read(serial_fd, recv_msg, sizeof(recv_msg)))) {
				fprintf(stderr, "Failed to read message!\n");
			} else {
				// recv_msg[recv_msg_len] = '\0';
				for (int i = 0; i < recv_msg_len; i++) {
					fprintf(stdout, "%c", recv_msg[i]);
				}
				/* Without a newline character, it is unlikely stdout will be
				 * flushed for this message, so manually flush */
				fflush(stdout);
			}

			FD_CLR(serial_fd, &rset);
		}

		/* If there is data to be read from stdin */
		if (FD_ISSET(STDIN_FILENO, &rset)) {
			/* Read a message from stdin */
			if (-1 == (send_msg_len = read(STDIN_FILENO, send_msg, sizeof(send_msg)))) {
				fprintf(stderr, "Failed to read user input!\n");
				FD_CLR(STDIN_FILENO, &rset);
			} else {
				FD_CLR(STDIN_FILENO, &rset);

				/* If we can write to the serial device, and we have a message to send */
				if (FD_ISSET(serial_fd, &wset)) {
					/* Send a message on the serial port through the GPIO */
					if (send_msg_len != write(serial_fd, send_msg, send_msg_len)) {
						fprintf(stderr, "Failed to send message!\n");
					}

					FD_CLR(serial_fd, &wset);
				}
			}
		}

	}

	close(serial_fd);

	return 0;
}
