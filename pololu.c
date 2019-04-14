/*
 * pololu.c
 *
 *  Created on: Mar. 13, 2019
 *      Author: mathieu
 */

#include "pololu.h"
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <pthread.h>

static int fdPololu = -1;
static pthread_mutex_t mutex;

#define POLOLU_PWM_PERIOD			2500

/*
 * Initalization of polulu UART device
 * -1 : opening error
 * -2 : getting attrbiute error
 */
int initPololu(const char* pPath)
{
	// init mutex
	pthread_mutex_init(&mutex, NULL);

	// open device
	fdPololu = open(pPath, O_RDWR | O_NOCTTY);

	if(fdPololu < 0)
	{
		perror("Error while opening device (pololu)");
		return fdPololu;
	}

	// configure device
	struct termios *options = (struct termios*) malloc(sizeof(struct termios));
	tcgetattr(fdPololu, options);

	if(!options)
	{
		perror("Error while getting attributes");
		return -2;
	}

	// configuring UART
	options->c_cflag = B9600 | CS8 | CLOCAL | CREAD; // baud = 9600, 8 bits
	options->c_iflag = IGNPAR; // no parity bit
	options->c_oflag = 0;
	options->c_lflag = 0;

	// flush and set attributes
	tcflush(fdPololu, TCIFLUSH);
	tcsetattr(fdPololu, TCSANOW, options);

	// no error
	free(options);
	return 0;
}


/*
 * Set position of channel X
 * return -1 if error while writing
 */
int setPosition(int channel, int positionUs)
{
	// lock mutex
	pthread_mutex_lock(&mutex);

	uint32_t target = (uint32_t)(positionUs*4);

	// setting command
	unsigned char command[] = {0x84, (unsigned char) channel, target & 0x7F, target >> 7 & 0x7F};

	// writing to device
	if(write(fdPololu, &command, sizeof(command)) < 0)
	{
		perror("Error while setting position of device");
		return -1;
	}

	// lock mutex
	pthread_mutex_unlock(&mutex);

	return 0;
}

/*
 * Set pwm of channel 8
 * return -1 if error while writing
 * duty cycle = 0 to 1.0
 */
int setPWM(double dutyCycle)
{
	// lock mutex
	pthread_mutex_lock(&mutex);

	uint32_t target = ((uint32_t) (dutyCycle * (double)POLOLU_PWM_PERIOD));
	uint32_t period = POLOLU_PWM_PERIOD;

	// setting command
	unsigned char command[] = {0x8A, target & 0x7F, target >> 7 & 0x7F, period & 0x7F, period >> 7 & 0x7F};

	// writing to device
	if(write(fdPololu, &command, sizeof(command)) < 0)
	{
		perror("Error while setting position of device");
		return -1;
	}

	// lock mutex
	pthread_mutex_unlock(&mutex);

	return 0;
}



/*
 * send home all channels
 */
int homing(void)
{
	// setting up command
	unsigned char command[] = {0xA2};

	// writing to device
	if(write(fdPololu, &command, sizeof(command)) < 0)
	{
		perror("Error while homing");
		return -1;
	}

	return 0;
}

/*
 * Set speed of channel X
 * return -1 if error while writing
 */
int setSpeed(int channel, int speedUsPerMs)
{
	// calculate speed
	double speedDouble = (double)(10)*((double)(speedUsPerMs)/(double)(0.25));
	uint32_t speed = (uint32_t) speedDouble;

	// setting up command
	unsigned char command[] = {0x87, (unsigned char) channel, speed & 0x7F, speed >> 7 & 0x7F};

	// writing to device
	if(write(fdPololu, &command, sizeof(command)) < 0)
	{
		perror("Error while setting speed");
		return -1;
	}

	return 0;
}

/*
 * Set acceleration of channel X
 * return -1 if error while writing
 */
int setAcceleration(int channel, int accelerationUsPerMsPerMs)
{
	//TODO
}

/*
 * Get position of resquested channel
 * return -1 if error while writing or reading
 */
int getPosition(int channel)
{
	//TODO
}

/*
 * Gett error on requested channel
 * return -1 if error while reading or writing
 */
int getErrors(int channel)
{
	//TODO
}


