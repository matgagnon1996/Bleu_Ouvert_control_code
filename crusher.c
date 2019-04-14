/*
 * crusher.c
 *
 *  Created on: Apr. 14, 2019
 *      Author: mathieu
 */

#include "crusher.h"

/*
 * init GPIO and SPI
 */
void initCrusherMotor()
{
	//TODO
	// init GPIO
	// set motor to brakes
}

/*
 * Stop crusher with a slope
 */
void stopCrusherMotor()
{
	//TODO
	// decrement speed and wait 1 second
	// decrement to 0
	// when 0 => brake (0 0)
}

/*
 * Start crusher with slope to max speed in FWD direction
 */
void startCrusherMotor(int speed)
{
	//TODO
	// set direction (1 0 or 0 1)? FWD
	// increment speed and wait 1 second
	// stop when max speed
}

/*
 * crusher is working in BWD direction at max speed SPEED
 */
void reverseCrusherMotor(int speed)
{
	//TODO
	// set direction (1 0 or 0 1)? BWD
	// increment speed and wait 1 second
	// stop when max speed
}

