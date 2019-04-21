/*
 * ADS1115.c
 *
 *  Created on: Mar. 24, 2019
 *      Author: mathieu
 */

#include "ADS1115.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <bcm2835.h>


#define SLV_ADDR_GROUND					0x48
#define SLV_ADDR_VDD					0x49
#define SLV_ADDR_SDA					0x4A
#define SLV_ADDR_SCL					0x4B

// change when chaging connection
#define SLV_ADDR						SLV_ADDR_GROUND
#define ADS1115_DEFAULT_MODE			0x0183
#define ADS1115_CHANNEL0_MODE			0xC183
#define ADS1115_CHANNEL1_MODE			0xD183
#define ADS1115_CHANNEL2_MODE			0xE183
#define ADS1115_CHANNEL3_MODE			0xF183

#define ADS1115_PGA_6144V				0x0000
#define ADS1115_PGA_4096V				0x0200
#define ADS1115_PGA_2048V				0x0400
#define ADS1115_PGA_1024V				0x0600
#define ADS1115_PGA_0512V				0x0800
#define ADS1115_PGA_0256V				0x0E00
#define ADS1115_DEFAULT_GAIN			ADS1115_PGA_4096V

#define ADS1115_SCALE_6144V				0.1875			//mv
#define ADS1115_SCALE_4096V				0.125			//mv
#define ADS1115_SCALE_2048V				0.0625			//mv
#define ADS1115_SCALE_1024V				0.03125			//mv
#define ADS1115_SCALE_0512V				0.015625		//mv
#define ADS1115_SCALE_0256V				0.0078125		//mv
#define ADS1115_DEFAULT_SCALE			ADS1115_SCALE_4096V

#define ADS1115_CONVERSION_REG_ADDR		0x0
#define ADS1115_CONFIG_REG_ADDR			0x1
#define ADS1115_LO_TRESH_REG_ADDR		0x2
#define ADS1115_HI_TRESH_REG_ADDR		0x3
#define ADS1115_REGISTER_LENGTH			2 // bytes

#define ADS1115_DEFAULT_BAUDRATE		100000
#define ADS1115_CONVERSION_DELAY		8000		// us

#define ADS1115_SLOPE_CRUSHER					35.654 //m
#define ADS1115_OFFSET_CRUSHER					-18.611 //b

#define ADS1115_SLOPE_EXTRUDER					11.111 //m
#define ADS1115_OFFSET_EXTRUDER					-29.8217 // b

/*
 * Init ads in default mode
 */
int initADS1115(void)
{
	// configure i2c pins
	bcm2835_i2c_begin();

	// configure slave addr
	bcm2835_i2c_setSlaveAddress(SLV_ADDR);

	// configure baudrate
	bcm2835_i2c_set_baudrate(ADS1115_DEFAULT_BAUDRATE);

	// write to config register the default config
	int error = writeRegisterADS1115(ADS1115_CONFIG_REG_ADDR, ADS1115_DEFAULT_MODE);

	if(error != BCM2835_I2C_REASON_OK)
	{
		perror("initADS1115() : Error while intializing I2C");
		return error;
	}

	// no error
	return 0;
}

/*
 * Read ADC value once
 *
 */
double readChannelValueADS1115(int channel)
{
	// start conversion on requested channel
	uint16_t config = 0;
	switch(channel)
	{
		case 0:
			config = ADS1115_CHANNEL0_MODE;
			break;

		case 1:
			config = ADS1115_CHANNEL1_MODE;
			break;

		case 2:
			config = ADS1115_CHANNEL2_MODE;
			break;

		case 3:
			config = ADS1115_CHANNEL3_MODE;
			break;

		default:
			return -1;
	}

	// chose gain
	config |= ADS1115_DEFAULT_GAIN;
	// write to config register
	int error = writeRegisterADS1115(ADS1115_CONFIG_REG_ADDR, config);

	if(error != BCM2835_I2C_REASON_OK)
	{
		perror("readChannelValueADS1115() : error while writing to configuration register");
	}

	// wait for conversion to finish
	usleep(ADS1115_CONVERSION_DELAY);

	// read conversion register
	uint16_t* readValue = (uint16_t*)malloc(sizeof(uint16_t));

	error = readRegisterADS1115(ADS1115_CONVERSION_REG_ADDR, readValue);

	if(error != BCM2835_I2C_REASON_OK)
	{
		perror("readChannelValueADS1115() : error while reading conversion register");
	}

	// invert MSB and LSB
	uint16_t revertedValue = (*readValue << 8) | (*readValue >> 8 );
	double voltageValue = ((double) ((revertedValue)*((double)ADS1115_DEFAULT_SCALE))/((double)1000.0));

	// free memory
	free(readValue);

	return voltageValue;
}

/*
 * convert voltage to current
 */
double voltageToCurrentCrusher(double v)
{
	return (double)(v*(double)(ADS1115_SLOPE_CRUSHER) + (double)(ADS1115_OFFSET_CRUSHER));
}

/*
 * convert voltage to current
 */
double voltageToCurrentExtruder(double v)
{
	return (double)(v*(double)(ADS1115_SLOPE_EXTRUDER) + (double)(ADS1115_OFFSET_EXTRUDER));
}

/*
 * read value of hardware register
 * return : BCM2835_I2C_REASON_OK if successful
 * return : error code if error occured
 */
int readRegisterADS1115(uint8_t regAddr, uint16_t* pReadValue)
{
	int error = 0;
	// write register address to pointer register
	error = bcm2835_i2c_write((const char*)(&regAddr),1);

	if(error != BCM2835_I2C_REASON_OK)
	{
		perror("readRegisterADS1115() : Error while writing to pointer register");
		return error;
	}

	// read register
	error = bcm2835_i2c_read((char*)pReadValue, sizeof(uint16_t));

	if(error != BCM2835_I2C_REASON_OK)
	{
		perror("readRegisterADS1115() : Error while reading register register");
		return error;
	}

	return BCM2835_I2C_REASON_OK;
}

/*
 * write value of hw register
 * return : BCM2835_I2C_REASON_OK if successful
 * return : error code if error occured
 */
int writeRegisterADS1115(uint8_t regAddr, uint16_t writeValue)
{
	// initiliazing data to send to device
	char* dataToSend = (char*)malloc(sizeof(uint16_t) + sizeof(uint8_t));
	dataToSend[0] = regAddr;
	dataToSend[1] = (writeValue >> 8) & 0xFF;
	dataToSend[2] = writeValue & 0xFF;

	// sending data to device
	int error = bcm2835_i2c_write((const char*)dataToSend, sizeof(uint16_t) + sizeof(uint8_t));

	if(error != BCM2835_I2C_REASON_OK)
	{
		perror("writeRegisterADS1115() : Error while writing to register");
		return error;
	}

	// free memory
	free(dataToSend);

	return BCM2835_I2C_REASON_OK;
}

int closeADS1115(void)
{
	// return i2c pin to default state
	bcm2835_i2c_end();
	return 0;
}




