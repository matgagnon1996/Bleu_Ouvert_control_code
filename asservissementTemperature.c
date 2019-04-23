/*
 * asservissementTemperature.c
 *
 *  Created on: Apr. 2, 2019
 *      Author: mathieu
 */

#include "asservissementTemperature.h"
#include "FSM_main.h"
#include "stdio.h"

#define COEFFICIANT1 			0.07075//0.07181
#define COEFFICIANT2			-0.07065//-0.07179

#define MIN_DUTY_CYCLE				0.00
#define MAX_DUTY_CYCLE				1.00

static volatile double command = 0.0;
static double tabAction[2] = {0.0, 0.0}; // Y(n)
static double tabErreur[2] = {0.0, 0.0}; // U(n)


double calculateActionToApply(double actualTemp)
{
	// calculate new error
	tabErreur[1] = tabErreur[0];
	tabErreur[0] = command - actualTemp;

	printf("error : %f \n", tabErreur[0]);
	printf("Command : %f \n ", command);
	// calculate new action
	tabAction[1] = tabAction[0];
	tabAction[0] = COEFFICIANT1*tabErreur[0] + COEFFICIANT2*tabErreur[1] + tabAction[1];

	// check if we saturate
	if(tabAction[0] > MAX_DUTY_CYCLE)
	{
		tabAction[0] = MAX_DUTY_CYCLE;
	}

	// check if command is negativ
	if(tabAction[0] < MIN_DUTY_CYCLE)
	{
		tabAction[0] = MIN_DUTY_CYCLE;
	}

	return tabAction[0];

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

