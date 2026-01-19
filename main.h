#if !defined __MAIN_H
#define __MAIN_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <linux/gpio.h>
#include <poll.h>

struct TCharDevice {
	char 	m_strFileIO[60];
	int	m_iDescIO;
};

struct TDeviceChar;
struct TDeviceChar {
	int 			m_iDescIo;
	const char 	m_strPathDev[62];
};

struct TDeviceCharGpIO;
struct TDeviceCharGpIO {
	struct TDeviceChar 			m_DevChar;
	struct gpiochip_info 		m_chipinfo;
	struct gpioline_info 		m_lineinfo;
	struct gpiohandle_request 	m_request;
	struct gpiohandle_data 		m_data;
	struct gpioevent_request  	m_event_request[64];
	struct gpioevent_data     	m_event_data[64];
	struct pollfd 					m_pollfd[64];
};

/* Function declarations */
int open_serial(const char* port);

#endif
