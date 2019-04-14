/*
 * readSPI.h
 *
 *  Created on: Apr. 12, 2019
 *      Author: mathieu
 */

#ifndef SPIDEVICE_H_
#define SPIDEVICE_H_

#include <unistd.h>

void initSPI();
void readTemperatureSPI(char*, size_t);
void setPotentiometerValueSPI(int);




#endif /* SPIDEVICE_H_ */
