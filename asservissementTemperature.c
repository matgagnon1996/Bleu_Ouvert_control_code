/*
 * asservissementTemperature.c
 *
 *  Created on: Apr. 2, 2019
 *      Author: mathieu
 */

#include "asservissementTemperature.h"
#include "FSM_main.h"
#include "stdio.h"

#define COEFFICIANT1 			0.5128//0.0612877 //
#define COEFFICIANT2			-0.5111

#define MIN_VOLTAGE				0.00
#define MAX_VOLTAGE				12.0

static volatile double command = 0.0;
static double tabAction[2] = {0.0, 0.0}; // Y(n)
static double tabErreur[2] = {0.0, 0.0}; // U(n)


double calculateActionToApply(double actualTemp)
{
	// calculate new error
	tabErreur[1] = tabErreur[0];
	tabErreur[0] = command - actualTemp;

	//printf("Erreur : %f \n", tabErreur[0]);

	// calculate new action
	tabAction[1] = tabAction[0];
	tabAction[0] = COEFFICIANT1*tabErreur[0] + COEFFICIANT2*tabErreur[1] + tabAction[1];

	// check if we saturate
	if(tabAction[0] > MAX_VOLTAGE)
	{
		tabAction[0] = MAX_VOLTAGE;
	}

	// check if command is negativ
	if(tabAction[0] < MIN_VOLTAGE)
	{
		tabAction[0] = MIN_VOLTAGE;
	}

	return tabAction[0];

}

double voltageToDutyCycle(double v)
{
	double dutyCycle = ((double)(v))/((double)(MAX_VOLTAGE));

	if(dutyCycle > 1.0)
	{
		return 1.0;
	}else if(dutyCycle < 0.0)
	{
		return 0.0;
	}else
	{
		return dutyCycle;
	}

}


void setTemperatureCommand()
{
	command = mainFsmStatus->requestedTemperature;
}

double getTemperatureCommand()
{
	return command;
}

void clearTemperatureCommand()
{
	// clear parameter to stop
	command = 0.0;
	tabAction[0] = 0.0;
	tabAction[1] = 0.0;
	tabErreur[0] = 0.0;
	tabErreur[1] = 0.0;
}

