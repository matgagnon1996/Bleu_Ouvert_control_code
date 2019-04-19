/*
 * readSPI.h
 *
 *  Created on: Apr. 12, 2019
 *      Author: mathieu
 */

#ifndef SPIDEVICE_H_
#define SPIDEVICE_H_

#include <unistd.h>
#include <stdint.h>

void initSPI();
void readTemperatureSPI(char*, size_t);
void setPotentiometerValueSPI(uint8_t);




#endif /* SPIDEVICE_H_ */
