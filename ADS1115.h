/*
 * ADS1115.h
 *
 *  Created on: Mar. 24, 2019
 *      Author: mathieu
 */

#include <stdint.h>

#ifndef ADS1115_H_
#define ADS1115_H_

#define CRUSHER_MOTOR_CURRENT_CHANNEL		0
#define EXTRUDER_MOTOR_CURRENT_CHANNEL		1

int initADS1115(void);
int closeADS1115(void);
double readChannelValueADS1115(int);

int readRegisterADS1115(uint8_t,uint16_t*);
int writeRegisterADS1115(uint8_t,uint16_t);
double voltageToCurrentCrusher(double v);
double voltageToCurrentExtruder(double v);

#endif /* ADS1115_H_ */
