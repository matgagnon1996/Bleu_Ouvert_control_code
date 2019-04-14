/*
 * FSM_extruder.c
 *
 *  Created on: Mar. 31, 2019
 *      Author: mathieu
 */

#include "FSM_extruder.h"
#include "FSM_main.h"
#include "pololu.h"
#include "crusher.h"
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <math.h>

#define FSM_EXTRUDER_IDLE_STATE								0
#define FSM_EXTRUDER_START_HEATER							1
#define FSM_EXTRUDER_WAIT_FOR_HEATER_STATE					2
#define FSM_EXTRUDER_START_CRUSHER_STATE					3
#define FSM_EXTRUDER_OPEN50_STATE							4
#define FSM_EXTRUDER_OPEN100_STATE							5
#define FSM_EXTRUDER_WAIT_FOR_WEIGHT_STATE					6
#define FSM_EXTRUDER_EMPTY_CONVOY_STATE						7
#define FSM_EXTRUDER_END_STATE								8
#define FSM_EXTRUDER_END_STATE_WITH_ERROR					9
#define FSM_EXTUDER_EMPTY_CONVOY_TO_GARBAGE_STATE			10

#define FSM_EXTRUDER_TERMINATED_OK							1
#define FSM_EXTRUDER_TERMINATED_ERROR						2

#define FSM_EXTRUDER_MOTOR_INCREMENT							100
#define FSM_EXTRUDER_MAX_WEIGHT_CONVOY						1000 // 1 kG
#define FSM_EXTRUDER_MAX_CRUSHER_CURRENT								2.0 // A
#define FSM_EXTRUDER_MIN_CRUSHER_CURRENT								1.5 // A
#define FSM_EXTRUDER_MAX_EXTRUDER_CURRENT								2.0 // A
#define FSM_EXTRUDER_MIN_EXTRUDER_CURRENT								1.5 // A
#define FSM_EXTRUDER_MAX_COUNT_NO_PLASTIC					120
#define FSM_EXTRUDER_TEMP_PRECISION							1.0
#define FSM_EXTRUDER_MAX_PERIODIC_REVERSE_COUNT				25

static int stateFsmExtruder = FSM_EXTRUDER_IDLE_STATE; //IDLE
static int returnValue = 0;
static double totalWeight = 0.0;
static int countNoPlastic = 0;
static int countPeriodicReverse = 0;

// start fsm or not
static volatile int fsmStarted = 0;

void startExtruderFSM()
{
	fsmStarted = 1;
}

void stopExtruderFSM()
{
	fsmStarted = 0;
}

int updateExtruderFSM(void)
{
	switch(stateFsmExtruder)
	{
		case FSM_EXTRUDER_IDLE_STATE:
			// wait for fsm started
			returnValue = 0;
			totalWeight = 0.0;
			countNoPlastic = 0;
			countPeriodicReverse = 0;
			setExtruderFsmState("En attente");

			if(fsmStarted == 1)
			{
				stateFsmExtruder = FSM_EXTRUDER_START_HEATER;
			}else
			{
				stateFsmExtruder = FSM_EXTRUDER_IDLE_STATE;
			}

			break;

		case FSM_EXTRUDER_START_HEATER:
			if(mainFsmStatus->enslavementStarted == 1)
			{
				// wait for heater
				stateFsmExtruder = FSM_EXTRUDER_WAIT_FOR_HEATER_STATE;
			}else
			{
				//start enslavement and wait
				mainFsmStatus->enslavementStarted = 1;
				stateFsmExtruder = FSM_EXTRUDER_WAIT_FOR_HEATER_STATE;
			}
			break;

		case FSM_EXTRUDER_WAIT_FOR_HEATER_STATE:

			if(fsmStarted == 1)
			{
				setExtruderFsmState("Chauffage de l'extrudeur");

				// wait for requestted temp
				if(fabs(mainFsmStatus->extruderTemperature - mainFsmStatus->requestedTemperature) < FSM_EXTRUDER_TEMP_PRECISION)
				{
					stateFsmExtruder = FSM_EXTRUDER_START_CRUSHER_STATE;
				}else
				{
					stateFsmExtruder = FSM_EXTRUDER_WAIT_FOR_HEATER_STATE;
				}
				break;
			}else
			{
				stateFsmExtruder = FSM_EXTRUDER_END_STATE;
				break;
			}


		case FSM_EXTRUDER_START_CRUSHER_STATE:
			// apply graduate input to motor
			setExtruderFsmState("Démarrage du moteur");
			startCrusherMotor(CRUSHER_FWD_SPEED);
			stateFsmExtruder = FSM_EXTRUDER_OPEN100_STATE;
			break;

		case FSM_EXTRUDER_OPEN100_STATE:
			// open distributor gate to 50%
			setPosition(POLOLU_DISTRIBUTOR_MOTOR, DISTRIBUTOR_MOTOR_OPEN);
			stateFsmExtruder = FSM_EXTRUDER_WAIT_FOR_WEIGHT_STATE;
			break;

		case FSM_EXTRUDER_WAIT_FOR_WEIGHT_STATE:
			if(fsmStarted == 1)
			{
				countPeriodicReverse++;
				// read weight
				setExtruderFsmState("remplissage du convoyeur");

				// check for bourrage
				if(mainFsmStatus->crusherCurrent > FSM_EXTRUDER_MAX_CRUSHER_CURRENT)
				{
					// il y a une blocage!
					bourrageRoutineExtruderFSM();
				}

				// check if there is enough plastic
				if(mainFsmStatus->crusherCurrent < FSM_EXTRUDER_MIN_CRUSHER_CURRENT)
				{
					// il y aurait possibilité de ne plus avoir de plastique!
					countNoPlastic++;
					if(countNoPlastic >= FSM_EXTRUDER_MAX_COUNT_NO_PLASTIC)
					{
						// il faut arrêter car on n'a plus de plastique
						stateFsmExtruder = FSM_EXTRUDER_END_STATE_WITH_ERROR;
						break;
					}
				}else
				{
					// Il ne manque pas de plastique
					countNoPlastic = 0;
				}

				// check if mold is full
				if(mainFsmStatus->extruderCurrent > FSM_EXTRUDER_MAX_EXTRUDER_CURRENT)
				{
					// mold is full
					stateFsmExtruder = FSM_EXTUDER_EMPTY_CONVOY_TO_GARBAGE_STATE;
					break;
				}

				// check for requested weight
				if(mainFsmStatus->convoyWeight > FSM_EXTRUDER_MAX_WEIGHT_CONVOY)
				{
					// max weight on convoy
					totalWeight += mainFsmStatus->convoyWeight;
					stateFsmExtruder = FSM_EXTRUDER_EMPTY_CONVOY_STATE;
					break;
				}

				// check if we have to do a periodic reverse
				if(countPeriodicReverse % FSM_EXTRUDER_MAX_PERIODIC_REVERSE_COUNT == 0)
				{
					// do a periodic reverse
					periodicReverseExtruderFSM();
				}

				stateFsmExtruder = FSM_EXTRUDER_WAIT_FOR_WEIGHT_STATE;
				break;
			}else
			{
				stateFsmExtruder = FSM_EXTRUDER_END_STATE;
				break;
			}

		case FSM_EXTRUDER_EMPTY_CONVOY_STATE:
			if(fsmStarted == 1)
			{
				// stop crusher motor and wait a bit
				setExtruderFsmState("Envoie à l'extrudeur");
				stopCrusherMotor();

				// empty convoy into distributor
				setPosition(POLOLU_CONVOY_MOTOR, CONVOY_MOTOR_BWD);
				sleep(5);
				setPosition(POLOLU_CONVOY_MOTOR, CONVOY_MOTOR_STOP);

				// restart motor
				startCrusherMotor(CRUSHER_FWD_SPEED);

				// return to wait for mold
				stateFsmExtruder = FSM_EXTRUDER_WAIT_FOR_WEIGHT_STATE;
				break;
			}else
			{
				stateFsmExtruder = FSM_EXTRUDER_END_STATE;
				break;
			}

		case FSM_EXTUDER_EMPTY_CONVOY_TO_GARBAGE_STATE:
			// stop crusher motor and wait a bit
			setExtruderFsmState("Vidage du convoyeur");
			stopCrusherMotor();

			// empty convoy into distributor
			setPosition(POLOLU_CONVOY_MOTOR, CONVOY_MOTOR_FWD);
			sleep(5);
			setPosition(POLOLU_CONVOY_MOTOR, CONVOY_MOTOR_STOP);

			stateFsmExtruder = FSM_EXTRUDER_END_STATE;
			break;

		case FSM_EXTRUDER_END_STATE:
			// stop all
			homing();
			stopCrusherMotor();
			fsmStarted = 0;
			returnValue = 1;
			stateFsmExtruder = FSM_EXTRUDER_IDLE_STATE;
			break;

		case FSM_EXTRUDER_END_STATE_WITH_ERROR:
			// stop all
			homing();
			stopCrusherMotor();
			fsmStarted = 0;
			returnValue = 2; // error
			stateFsmExtruder = FSM_EXTRUDER_IDLE_STATE;
			break;

		default:
			break;

	}

	return returnValue;
}

void bourrageRoutineExtruderFSM()
{
	setExtruderFsmState("Blocage");
	countPeriodicReverse = 1;
	mainFsmStatus->crusherIsBlocked = 1;
	// stop motor and sleep 1 seconds
	stopCrusherMotor();
	sleep(1);

	// rotate motor in other direction (not at max speed
	reverseCrusherMotor(CRUSHER_BWD_SPEED);
	sleep(5);

	// stop motor
	stopCrusherMotor();
	sleep(1);

	// start motor with slope
	startCrusherMotor(CRUSHER_FWD_SPEED);
	mainFsmStatus->crusherIsBlocked = 0;
}

void setExtruderFsmState(char* strValue)
{
	memset(mainFsmStatus->fsmExtruderStatus, 0, 50);
	memcpy(mainFsmStatus->fsmExtruderStatus, strValue, strlen(strValue));
}

void periodicReverseExtruderFSM()
{
	setExtruderFsmState("Inversion périodique du déchiqueteur");

	// stop crusher and run it in BWD direction
	stopCrusherMotor();
	reverseCrusherMotor(CRUSHER_BWD_SPEED);

	// wait for 5 seconds
	sleep(5);

	// stop crusher again
	stopCrusherMotor();
	startCrusherMotor(CRUSHER_FWD_SPEED);
}


