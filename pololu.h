/*
 * pololu.h
 *
 *  Created on: Mar. 13, 2019
 *      Author: mathieu
 */

#ifndef POLOLU_H_
#define POLOLU_H_

#include <stdint.h>

typedef struct
{
	uint32_t channel;		// channel
	uint32_t cmd_type;		// set position, accleration or speed
	uint32_t value;			// value in us
}t_pololu_cmd;

#define POLOLU_CMD_POSITION		0
#define POLOLU_CMD_SPEED		1
#define POLOLU_CMD_ACCELERATION	2

// position in us
#define CONVOY_MOTOR_STOP			1300
#define CONVOY_MOTOR_FWD			1750
#define CONVOY_MOTOR_BWD			750

#define EXTRUDER_MOTOR_STOP			1000
#define EXTRUDER_MOTOR_RUN			2000

#define EXTRUDER_HEATER_STOP		1000
#define EXTRUDER_HEATER_RUN			2000

#define DISTRIBUTOR_MOTOR_CLOSE		1000
#define DISTRIBUTOR_MOTOR_OPEN		2000
#define DISTRIBUTOR_MOTOR_OPEN50	1500

#define CRUSHER_MOTOR_FWD			2000
#define CRUSHER_MOTOR_BWD 			1000
#define CRUSHER_MOTOR_IDLE			1500

//#define
#define POLOLU_CRUSHER_MOTOR					0
#define POLOLU_EXTRUDER_MOTOR					1 // PWM
#define POLOLU_CRUSHER_MOTOR_CTRL				2 // PWM
#define POLOLU_EXTRUDER_HEATER					8 // PWM
#define POLOLU_CONVOY_MOTOR						3
#define POLOLU_DISTRIBUTOR_MOTOR				4
#define POLOLU_DISTRIBUTOR_MOTOR_INV			5


int initPololu(const char*);
int setPosition(int,int);
int homing(void);
int setSpeed(int, int);
int setAcceleration(int, int);
int getPosition(int);
int getErrors(int);
int setPWM(double);



#endif /* POLOLU_H_ */
