#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <linux/uinput.h>

// Communication messages
char TRANS_INIT[3] = {0xFF, 0x07, 0x03};
char TRANS_REQUEST_ACC[7] = {0xFF, 0x08, 0x07, 0x00, 0x00, 0x00, 0x00};

// Definitions
#define MOUSE_MODE 0
#define JOYSTICK_MODE 1
#define die(str, args...) do { \
        perror(str); \
        exit(EXIT_FAILURE); \
    } while(0)

// Default variables
char* accessPointDev = "/dev/uinput";
char* uInputDev = "/dev/ttyACM0";
int delay = 30000;
int devmode = MOUSE_MODE;
double accelX = 1, accelY = 1, accelZ = 1;
int swapXZ = 0;

// Global variables
int fd_uinput, fd_transmitter;

void parseCmdOptions(int argc, char **argv) {
	int option;
	while ((option = getopt(argc, argv, "a:u:p:m:s:xh")) != -1) {
		switch (option) {
		case 'a':
			accessPointDev = optarg;
			break;
		case 'u':
			uInputDev = optarg;
			break;
		case 'p':
			delay = atoi(optarg);
			break;
		case 'm':
			if (optarg[0] == 'm') {
				devmode = MOUSE_MODE;
			} else if (optarg[0] == 'j') {
				devmode = JOYSTICK_MODE;
			} else {
				fprintf(stderr, "Error: Unknown mode '%c'\n", optarg[0]);
				exit(1);
			}
			break;
		case 's':
			if (optarg[0] == 'x') {
				accelX = atof(optarg + 1);
				printf("Setting acceleration of X axis to %f\n", accelX);
			} else if (optarg[0] == 'y') {
				accelY = atof(optarg + 1);
				printf("Setting acceleration of Y axis to %f\n", accelY);
			} else if (optarg[0] == 'z') {
				accelZ = atof(optarg + 1);
				printf("Setting acceleration of Z axis to %f\n", accelZ);
			} else {
				accelX = atof(optarg);
				accelY = atof(optarg);
				accelZ = atof(optarg);
				printf("Setting acceleration of all axes to %f\n", accelX);
			}
			break;
		case 'x':
			swapXZ = 1;
			break;
		case 'h':
		case '?':
			printf("\neZ430-Chronos mouse and joystick driver\n");
			printf("(c) Ignaz Forster; licensed under BSD license\n");
			printf("\nUsage:\n");
			printf("-m [m|j]\t\tUse watch as mouse or joystick device (default: mouse)\n");
			printf("-a <device>\t\tPath to access point (default: %s)\n", accessPointDev);
			printf("-u <device>\t\tPath to uInput device (default: %s)\n", uInputDev);
			printf("-p <microseconds>\tPolling interval (default: %d)\n", delay);
			printf("-s [x|y|z|]<multiplier>\tAccelerate axis / general movement speed (default: 1.0)\n");
			printf("-x\t\tJoystick mode: Swap X / Z axis (for racing wheel emulation");
			exit(0);
		}
	}
}

void sighandler(int sig)
{

	if (fd_transmitter)
		close(fd_transmitter);
	if (fd_uinput) {
		ioctl(fd_uinput, UI_DEV_DESTROY);
		close(fd_uinput);
	}
	printf("\nGoodbye!\n");
	exit(0);
}

void setupSignalHandlers()
{
	signal(SIGQUIT, sighandler);
	signal(SIGINT, sighandler);
}

void openDevices(char* accessPointDev, char* uInputDev)
{
	fd_uinput = open(uInputDev, O_WRONLY | O_NONBLOCK);
	if (fd_uinput < 0) {
		fd_uinput = open("/dev/input/uinput", O_WRONLY | O_NONBLOCK);
		if (fd_uinput < 0)
			die("Error: Could not open uinput device. Is the uinput driver loaded (modprobe uinput) and do you have write permissions for the device?");
	}

	fd_transmitter = open(accessPointDev, O_RDWR);
	if (fd_transmitter < 0)
		die("Error: Could not open access point device. Is the transmitter connected and do you have read and write permissions for the device?");
}

void setIoctlPropertiesForUinput()
{
	if (ioctl(fd_uinput, UI_SET_EVBIT, EV_KEY) < 0)
		die("Error: ioctl EV_KEY failed");
	if (devmode == MOUSE_MODE) {
		// Button events
		if (ioctl(fd_uinput, UI_SET_KEYBIT, BTN_LEFT) < 0)
			die("Error: ioctl BTN_LEFT failed");
		if (ioctl(fd_uinput, UI_SET_KEYBIT, BTN_RIGHT) < 0)
			die("Error: ioctl BTN_RIGHT failed");
		if (ioctl(fd_uinput, UI_SET_KEYBIT, BTN_MIDDLE) < 0)
			die("Error: ioctl BTN_MIDDLE failed");

		// Movement events
		if (ioctl(fd_uinput, UI_SET_EVBIT, EV_REL) < 0)
			die("Error: ioctl EV_REL failed");
		if (ioctl(fd_uinput, UI_SET_RELBIT, REL_X) < 0)
			die("Error: ioctl REL_X failed");
		if (ioctl(fd_uinput, UI_SET_RELBIT, REL_Y) < 0)
			die("Error: ioctl REL_Y failed");
	} else if (devmode == JOYSTICK_MODE) {
		if (ioctl(fd_uinput, UI_SET_KEYBIT, BTN_JOYSTICK) < 0)
			die("Error: ioctl BTN_LEFT failed");
		if (ioctl(fd_uinput, UI_SET_KEYBIT, BTN_JOYSTICK + 1) < 0)
			die("Error: ioctl BTN_LEFT failed");
		if (ioctl(fd_uinput, UI_SET_KEYBIT, BTN_JOYSTICK + 2) < 0)
			die("Error: ioctl BTN_LEFT failed");
		
		if (ioctl(fd_uinput, UI_SET_EVBIT, EV_ABS) < 0)
			die("Error: ioctl EV_REL failed");
		if (ioctl(fd_uinput, UI_SET_ABSBIT, ABS_X) < 0)
			die("Error: ioctl REL_X failed");
		if (ioctl(fd_uinput, UI_SET_ABSBIT, ABS_Y) < 0)
			die("Error: ioctl REL_Y failed");
		if (ioctl(fd_uinput, UI_SET_ABSBIT, ABS_Z) < 0)
			die("Error: ioctl REL_Y failed");
	}
}

struct uinput_user_dev createUinputDevice()
{
	struct uinput_user_dev uidev;

	memset(&uidev, 0, sizeof(uidev));
	snprintf(uidev.name, UINPUT_MAX_NAME_SIZE, "Texas Instruments eZ430-Chronos");
	uidev.id.bustype = BUS_USB;
	uidev.id.vendor = 0x0451;
	uidev.id.product = 0x16a6;
	uidev.id.version = 1;

	if (write(fd_uinput, &uidev, sizeof(uidev)) < 0)
		die("Error: Writing control structure for uinput device failed");

	if (ioctl(fd_uinput, UI_DEV_CREATE) < 0)
		die("Error: Creation of uinput device failed");

	return uidev;
}

void initAccessPoint(char* buffer)
{
	if (write(fd_transmitter, TRANS_INIT, sizeof(TRANS_INIT)) < 0)
		die("Error: Starting Access Point failed");
	// Ignore initialization data
	read(fd_transmitter, buffer, 3);
}

void getChronosEvents(char* buffer)
{
	if (write(fd_transmitter, TRANS_REQUEST_ACC, sizeof(TRANS_REQUEST_ACC)) < 0)
		die("Error: Requesting acceleration data failed");
	if (read(fd_transmitter, buffer, 7) < 0) {
		die("Error: Reading acceleration data failed");
	}
}

void sendInputEvent(__u16 type, __u16 code, __s32 value)
{
	struct input_event ev;
	memset(&ev, 0, sizeof(struct input_event));
	ev.type = type;
	ev.code = code;
	ev.value = value;
	if (write(fd_uinput, &ev, sizeof(struct input_event)) < 0)
		die("Error: Setting mouse event failed");
}

int main(int argc, char **argv)
{
	struct uinput_user_dev uidev;
	char buffer[7];
	__u16 button_pressed = 0;
	int debounce_counter = 0;
	char tmp;

	parseCmdOptions(argc, argv);			

	setupSignalHandlers();
	openDevices(uInputDev, accessPointDev);
	setIoctlPropertiesForUinput();
	uidev = createUinputDevice(devmode);

	// According to the manual we should wait up to 1 second after
	// initializing the Chronos hardware
	sleep(1);

	initAccessPoint(buffer);

	while (1) {
		getChronosEvents(buffer);

		//printf("RAW data: %d %d %d %d %d %d %d\n", buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], buffer[5], buffer[6]);

		if (button_pressed > 0) {
			sendInputEvent(EV_KEY, button_pressed, 0);
			button_pressed = 0;
			// Process button events and prevent bouncing
			// Unfortunately the watch only sends events when a button is
			// pressed, but not when it is released, so drag'n'drop cannot
			// be supported. Additionally ignore all movement data as
			// sending both a mouse click and a movement without a sync in
			// between results in a drag'n'drop action.
			// TODO: Find a better use for the keys in PPT mode
		} else if (buffer[3] > 1 && debounce_counter == 0) {
			if (devmode == MOUSE_MODE) {
				if (buffer[3] == 17 || buffer[3] == 18) {
					button_pressed = BTN_LEFT;
				} else if (buffer[3] == 49 || buffer[3] == 50) {
					button_pressed = BTN_RIGHT;
				} else if (buffer[3] == 33 || buffer[3] == 34) {
					button_pressed = BTN_MIDDLE;
				}
			} else if (devmode == JOYSTICK_MODE) {
				if (buffer[3] == 17 || buffer[3] == 18) {
					button_pressed = BTN_JOYSTICK;
				} else if (buffer[3] == 49 || buffer[3] == 50) {
					button_pressed = BTN_JOYSTICK + 1;
				} else if (buffer[3] == 33 || buffer[3] == 34) {
					button_pressed = BTN_JOYSTICK + 2;
				}
			}
			sendInputEvent(EV_KEY, button_pressed, 1);
			debounce_counter = 1;
			// Process acceleration data
		} else if (buffer[3] == 1) {
			if (devmode == MOUSE_MODE) {
				sendInputEvent(EV_REL, REL_X, buffer[5] * accelX);
				sendInputEvent(EV_REL, REL_Y, buffer[4] * accelY);
			} else if (devmode == JOYSTICK_MODE) {
				if (swapXZ) {
					tmp = buffer[6];
					buffer[6] = buffer[5];
					buffer[5] = tmp;
				}
				sendInputEvent(EV_ABS, ABS_X, buffer[5] * 256 * accelX);
				sendInputEvent(EV_ABS, ABS_Y, buffer[4] * 256 * accelY);
				sendInputEvent(EV_ABS, ABS_Z, buffer[6] * 256 * accelZ);
			}
		}
		sendInputEvent(EV_SYN, 0, 0);

		if (debounce_counter > 0) {
			if (debounce_counter == 20)
				debounce_counter = 0;
			else
				debounce_counter++;
		}

		usleep(delay);
	}

	return 0;
}

