/*
 * communication.h
 *
 *  Created on: Feb. 18, 2019
 *      Author: mathieu
 */

#include <stdint.h>

#ifndef COMMUNICATION_H_
#define COMMUNICATION_H_

struct RPIMessageHeader {
	char Signature[4];	// RPI\0
	uint32_t PayloadType; // 0 = command, 1 = error message, 2 = data (temperature, current, printer)
	uint32_t PayloadSize; // size of incoming data (bytes)
	uint32_t PayloadSource; // 0 = Main_PI (interface), 1 = math-pi-1, 2 = main-pi-2
};

typedef struct RPIMessageHeader RPIMessageHeader_t;

int sendMessage(int, char*, int, int, int);
int receiveMessage(int, RPIMessageHeader_t*, char*);

#define ACK_MSG					0  	// acknowledge command
#define NACK_MSG				1	// cannot acknowledge command
#define ID_MSG					2  	// identify client after conenction
#define ERROR_MSG				3	// error message
#define STS_TEMPERATURE_MSG		20  // sending temeprature data
#define STS_CRUSHER_BLOCKED_MSG	21	// sending current of motor
#define STS_WEIGHT_MSG			22 // sending actual weight
#define STS_FSM_MSG				23 // others
#define MANUAL_CMD_SERVO		30
#define CRUSHER_MODE_CMD		31
#define EXTRUDER_MODE_CMD		32
#define HEATER_MANUAL_CMD		33
#define STOP_CMD				34

#define RPI_CONTROL_ID			1
#define RPI_PRINTER_ID			2

//TODO

#endif /* COMMUNICATION_H_ */
