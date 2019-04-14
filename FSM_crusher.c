/*
 * FSM_crusher.c
 *
 *  Created on: Mar. 31, 2019
 *      Author: mathieu
 */

#include "FSM_crusher.h"
#include "FSM_main.h"
#include "pololu.h"
#include "crusher.h"
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define FSM_CRUSHER_IDLE_STATE								0
#define FSM_CRUSHER_START_CRUSHER_STATE						1
#define FSM_CRUSHER_OPEN50_STATE							2
#define FSM_CRUSHER_OPEN100_STATE							3
#define FSM_CRUSHER_WAIT_FOR_WEIGHT_STATE					4
#define FSM_CRUSHER_EMPTY_CONVOY_STATE						5
#define FSM_CRUSHER_END_STATE								6
#define FSM_CRUSHER_END_STATE_WITH_ERROR					7

#define FSM_CRUSHER_TERMINATED_OK							1
#define FSM_CRUSHER_TERMINATED_ERROR						2

#define FSM_CRUSHER_MOTOR_INCREMENT							100
#define FSM_CRUSHER_MAX_WEIGHT_CONVOY						1000 // 1 kG
#define FSM_CRUSHER_MAX_CURRENT								2.0 // A
#define FSM_CRUSHER_MIN_CURRENT								1.5 // A
#define FSM_CRUSHER_MAX_COUNT_NO_PLASTIC					120
#define FSM_CRUSHER_MAX_PERIODIC_REVERSE_COUNT				25


static int stateFsmCrusher = FSM_CRUSHER_IDLE_STATE; //IDLE
static int returnValue = 0;
static double totalWeight = 0.0;
static int countNoPlastic = 0;
static int countPeriodicReverse = 0; // reverse when count == 10

// start fsm or not
static volatile int fsmStarted = 0;

void startCrusherFSM()
{
	fsmStarted = 1;
}

void stopCrusherFSM()
{
	fsmStarted = 0;
}

int updateCrusherFSM(void)
{
	switch(stateFsmCrusher)
	{
		case FSM_CRUSHER_IDLE_STATE:
			// wait for fsm started
			returnValue = 0;
			totalWeight = 0.0;
			countNoPlastic = 0;
			countPeriodicReverse = 0;
			setCrusherFsmState("En attente");

			if(fsmStarted == 1)
			{
				stateFsmCrusher = FSM_CRUSHER_START_CRUSHER_STATE;
			}else
			{
				stateFsmCrusher = FSM_CRUSHER_IDLE_STATE;
			}

			break;

		case FSM_CRUSHER_START_CRUSHER_STATE:
			// apply graduate input to motor
			setCrusherFsmState("Démarrage du moteur");
			startCrusherMotor(CRUSHER_FWD_SPEED);
			stateFsmCrusher = FSM_CRUSHER_OPEN100_STATE;
			break;

		case FSM_CRUSHER_OPEN100_STATE:
			// open distributor gate to 50%
			setPosition(POLOLU_DISTRIBUTOR_MOTOR, DISTRIBUTOR_MOTOR_OPEN);
			stateFsmCrusher = FSM_CRUSHER_WAIT_FOR_WEIGHT_STATE;
			break;

		case FSM_CRUSHER_WAIT_FOR_WEIGHT_STATE:
			// read weight
			if(fsmStarted == 1)
			{
				countPeriodicReverse++;
				setCrusherFsmState("remplissage du convoyeur");

				// check for bourrage
				if(mainFsmStatus->crusherCurrent > FSM_CRUSHER_MAX_CURRENT)
				{
					// il y a une blocage!
					bourrageRoutineCrusherFSM();
				}

				// check if there plasi
				if(mainFsmStatus->crusherCurrent < FSM_CRUSHER_MIN_CURRENT)
				{
					// il y aurait possibilité de ne plus avoir de plastique!
					countNoPlastic++;
					if(countNoPlastic >= FSM_CRUSHER_MAX_COUNT_NO_PLASTIC)
					{
						// il faut arrêter car on n'a plus de plastique
						stateFsmCrusher = FSM_CRUSHER_END_STATE_WITH_ERROR;
						break;
					}
				}else
				{
					// Il ne manque pas de plastique
					countNoPlastic = 0;
				}

				// check for requested weight
				if(mainFsmStatus->convoyWeight > FSM_CRUSHER_MAX_WEIGHT_CONVOY || (totalWeight + mainFsmStatus->convoyWeight) > mainFsmStatus->requestedWeight)
				{
					// max weight on convoy
					totalWeight += mainFsmStatus->convoyWeight;
					stateFsmCrusher = FSM_CRUSHER_EMPTY_CONVOY_STATE;
					break;
				}

				// check if we have to do a periodic reverse
				if(countPeriodicReverse % FSM_CRUSHER_MAX_PERIODIC_REVERSE_COUNT == 0)
				{
					// do a periodic reverse
					periodicReverseCrusherFSM();
				}

				stateFsmCrusher = FSM_CRUSHER_WAIT_FOR_WEIGHT_STATE;
				break;
			}else
			{
				stateFsmCrusher = FSM_CRUSHER_END_STATE;
				break;
			}


		case FSM_CRUSHER_EMPTY_CONVOY_STATE:
			if(fsmStarted == 1)
			{
				// stop crusher motor and wait a bit
				setCrusherFsmState("Vidage du convoyeur");
				stopCrusherMotor();

				// empty convoy into distributor
				setPosition(POLOLU_CONVOY_MOTOR, CONVOY_MOTOR_FWD);
				sleep(5);
				setPosition(POLOLU_CONVOY_MOTOR, CONVOY_MOTOR_STOP);

				//restart motor and wait for weight
				if(totalWeight >= mainFsmStatus->requestedWeight)
				{
					stateFsmCrusher = FSM_CRUSHER_END_STATE;
				}else
				{
					stateFsmCrusher = FSM_CRUSHER_START_CRUSHER_STATE;
				}
				break;
			}else
			{
				stateFsmCrusher = FSM_CRUSHER_END_STATE;
				break;
			}

		case FSM_CRUSHER_END_STATE:
			// stop all
			homing();
			stopCrusherMotor();
			fsmStarted = 0;
			returnValue = 1;
			stateFsmCrusher = FSM_CRUSHER_IDLE_STATE;
			break;

		case FSM_CRUSHER_END_STATE_WITH_ERROR:
			// stop all
			homing();
			stopCrusherMotor();
			fsmStarted = 0;
			returnValue = 2; // error
			stateFsmCrusher = FSM_CRUSHER_IDLE_STATE;
			break;

		default:
			break;

	}

	return returnValue;
}

void bourrageRoutineCrusherFSM()
{
	setCrusherFsmState("Blocage");
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

void periodicReverseCrusherFSM()
{
	setCrusherFsmState("Inversion périodique du déchiqueteur");

	// stop crusher and run it in BWD direction
	stopCrusherMotor();
	reverseCrusherMotor(CRUSHER_BWD_SPEED);

	// wait for 5 seconds
	sleep(5);

	// stop crusher again
	stopCrusherMotor();
	startCrusherMotor(CRUSHER_FWD_SPEED);
}

void setCrusherFsmState(char* strValue)
{
	memset(mainFsmStatus->fsmCrusherStatus, 0, 50);
	memcpy(mainFsmStatus->fsmCrusherStatus, strValue, strlen(strValue));
}


