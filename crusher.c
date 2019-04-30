/*
 * crusher.c
 *
 *  Created on: Apr. 14, 2019
 *      Author: mathieu
 */

#include "crusher.h"
#include "bcm2835.h"
#include "stdio.h"
#include "unistd.h"
#include "SPIDevice.h"
#include "pololu.h"

#define CRUSHER_CTRL_PIN1			RPI_V2_GPIO_P1_35
#define CRUSHER_POT_MAX_RESISTANCE	10000
#define CRUSHER_POT_MIN_RESISTANCE	60
#define CRUSHER_SPEED_INCREMENT		10

/*
 * init GPIO and SPI
 */
void initCrusherMotor()
{
	//TODO
	// init GPIO
	bcm2835_gpio_fsel(CRUSHER_CTRL_PIN1, BCM2835_GPIO_FSEL_OUTP);

	// Init motor direction
	setPosition(POLOLU_CRUSHER_MOTOR_CTRL, CRUSHER_MOTOR_FWD);

	// init motor speed
	setPotentiometerValueSPI(0);

	sleep(1);
}

/*
 * Stop crusher with a slope
 */
void stopCrusherMotor(int fromSpeed)
{
	// decrement speed and wait 1 second
	int i = 0;
	for(i = fromSpeed; i >= 0 ; i-=CRUSHER_SPEED_INCREMENT)
	{
		// set pot value to decrease speed
		uint8_t potValue = ((int)(((double)(i)/ (double)(100.0))*255));
		setPotentiometerValueSPI(potValue);

		// wait
		sleep(1);
	}

	// brake motor
	setPosition(POLOLU_CRUSHER_MOTOR_CTRL, CRUSHER_MOTOR_FWD);
	sleep(1);
}

/*
 * Start crusher with slope to max speed in FWD direction
 */
void startCrusherMotor(int speed)
{
	// set direction FWD
	setPosition(POLOLU_CRUSHER_MOTOR_CTRL, CRUSHER_MOTOR_FWD);
	sleep(1);

	// increment speed and wait 2 second
	int i = 0;
	for(i = 0; i <= speed ; i+=CRUSHER_SPEED_INCREMENT)
	{
		// set pot value to decrease speed
		uint8_t potValue = ((int)(((double)(i)/ (double)(100.0))*255));//*((double)(1.0) / (double)(10.0));
		setPotentiometerValueSPI(potValue);

		// wait
		sleep(1);
	}

}

/*
 * crusher is working in BWD direction at max speed SPEED
 */
void reverseCrusherMotor(int speed)
{
	// set direction  BWD
	setPosition(POLOLU_CRUSHER_MOTOR_CTRL, CRUSHER_MOTOR_BWD);
	sleep(1);

	// increment speed and wait 2 second
	int i = 0;
	for(i = 0; i <= speed ; i+=CRUSHER_SPEED_INCREMENT)
	{
		// set pot value to decrease speed
		uint8_t potValue = ((int)(((double)(i)/ (double)(100.0))*255));//*((double)(1.0) / (double)(10.0));
		setPotentiometerValueSPI(potValue);

		// wait
		sleep(1);
	}

}

