/*
 * readSPI.c
 *
 *  Created on: Apr. 12, 2019
 *      Author: mathieu
 */


#include "SPIDevice.h"

#include <bcm2835.h>
#include <pthread.h>

static pthread_mutex_t mutexSPI;

void initSPI()
{
	// init mutex
	pthread_mutex_init(&mutexSPI, NULL);

	// init SPI device
	bcm2835_spi_begin();

	// set CS pin polarity
	bcm2835_spi_setChipSelectPolarity(BCM2835_SPI_CS0, 0);
	bcm2835_spi_setChipSelectPolarity(BCM2835_SPI_CS1, 0);

	// set SPI clock
	bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_512); // clock to 488.28 KHz

	// set data mode
	bcm2835_spi_setDataMode(BCM2835_SPI_MODE1);

	// set bit order to MSB first
	bcm2835_spi_setBitOrder(BCM2835_SPI_BIT_ORDER_MSBFIRST);

}



void readTemperatureSPI(char* readValue, size_t n)
{
	// lock mutex
	pthread_mutex_lock(&mutexSPI);

	// set chip select to use for temperature transfer
	bcm2835_spi_chipSelect(BCM2835_SPI_CS0);

	// read spi
	bcm2835_spi_transfern(readValue, sizeof(uint16_t));

	// unlock mutex
	pthread_mutex_unlock(&mutexSPI);
}

void setPotentiometerValueSPI(uint8_t value)
{
	// lock mutex
	pthread_mutex_lock(&mutexSPI);

	// set chip select to use for temperature transfer
	bcm2835_spi_chipSelect(BCM2835_SPI_CS1);

	// TODO : transert according to datasheet
	bcm2835_spi_transfer(value);

	// unlock mutex
	pthread_mutex_unlock(&mutexSPI);

}

