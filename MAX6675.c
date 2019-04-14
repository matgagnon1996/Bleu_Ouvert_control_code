/*
 * MAX6675.c
 *
 *  Created on: Mar. 24, 2019
 *      Author: mathieu
 */

#include "MAX6675.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <bcm2835.h>

#define MAX6675_TEMPERATURE_RESOLUTION			0.25 // degrees C/bit

#define MAX6675_OPEN_THERMOCOUPLE_MASK			0x0004
#define MAX6675_TEMPERATURE_MASK				0x0FFF
#define MAX6675_TEMPARTURE_BIT_SHIFT			3

int initMAX6675(void)
{
	initSPI();

	return 0;
}

int closeMAX6675(void)
{
	bcm2835_spi_end();

	return 0;
}

double readTemperatureMAX6675(void)
{
	// init transert value
	char readValue[] = {0x0, 0x0};

	// read spi
	readTemperatureSPI(&readValue[0], sizeof(uint16_t));

	uint16_t rawData = (readValue[0] << 8) + readValue[1];
	rawData = rawData >> MAX6675_TEMPARTURE_BIT_SHIFT;

	// convert to temperature
	double temperature = ((double)((rawData) & MAX6675_TEMPERATURE_MASK))*((double)MAX6675_TEMPERATURE_RESOLUTION);/// (double)(10.0);

	return temperature;
}




