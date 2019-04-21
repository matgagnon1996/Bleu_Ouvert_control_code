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
	//TODO
	// decrement speed and wait 1 second
	// decrement to 0
	int i = 0;
	for(i = fromSpeed; i >= 0 ; i-=CRUSHER_SPEED_INCREMENT)
	{
		// set pot value to decrease speed
		uint8_t potValue = ((int)(((double)(i)/ (double)(100.0))*255));//*((double)(1.0) / (double)(10.0));
		setPotentiometerValueSPI(potValue);

		// wait
		sleep(2);
	}

	// when 0 => brake (0 0)
	setPosition(POLOLU_CRUSHER_MOTOR_CTRL, CRUSHER_MOTOR_FWD);
	sleep(1);
}

/*
 * Start crusher with slope to max speed in FWD direction
 */
void startCrusherMotor(int speed)
{
	//TODO
	// set direction (1 0 or 0 1)? FWD
	setPosition(POLOLU_CRUSHER_MOTOR_CTRL, CRUSHER_MOTOR_FWD);
	sleep(1);

	// increment speed and wait 1 second
	int i = 0;
	for(i = 0; i <= speed ; i+=CRUSHER_SPEED_INCREMENT)
	{
		// set pot value to decrease speed
		uint8_t potValue = ((int)(((double)(i)/ (double)(100.0))*255));//*((double)(1.0) / (double)(10.0));
		setPotentiometerValueSPI(potValue);

		// wait
		sleep(2);
	}

}

/*
 * crusher is working in BWD direction at max speed SPEED
 */
void reverseCrusherMotor(int speed)
{
	//TODO
	printf("allo je tourne de bord! \n");
	// set direction (1 0 or 0 1)? BWD
	setPosition(POLOLU_CRUSHER_MOTOR_CTRL, CRUSHER_MOTOR_BWD);
	sleep(1);

	// increment speed and wait 1 second
	int i = 0;
	for(i = 0; i <= speed ; i+=CRUSHER_SPEED_INCREMENT)
	{
		// set pot value to decrease speed
		uint8_t potValue = ((int)(((double)(i)/ (double)(100.0))*255));//*((double)(1.0) / (double)(10.0));
		setPotentiometerValueSPI(potValue);

		// wait
		sleep(2);
	}

}

