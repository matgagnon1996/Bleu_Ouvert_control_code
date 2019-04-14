/*
 * main.c
 *
 *  Created on: Feb. 16, 2019
 *      Author: mathieu
 */

//debug defines
//#define _DEBUG
#define _CONNECTED
//#define _ADC_CONNECTED
#define _POLOLU_CONNECTED
#define _TEMP_CONNECTED
//#define _SCALE_CONNECTED

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <bcm2835.h>
#include <errno.h>
#include <pthread.h>
#include <time.h>

#include "clientPi.h"
#include "communication.h"
#include "pololu.h"
#include "ADS1115.h"
#include "MAX6675.h"
#include "FSM_main.h"
#include "hx711.h"
#include "FSM_crusher.h"
#include "FSM_extruder.h"
#include "asservissementTemperature.h"

// boolean to stop thread
static volatile int terminated = 0;

// structure for scale control
static volatile HX711 *hx711Sruct = 0;

// socket for command port and status port
static int socketFdCmd = -1;
static int socketFdSts = -1;

// threads declaration
pthread_t statusThread;
pthread_t commandThread;
pthread_t tempThread;
pthread_t fsmThread;

// thread function declaration
void *statusThreadFunction(void*);
void *commandThreadFunction(void*);
void *asservissementTemperatureThreadFunction(void*);
void *FSMThreadFunction(void* param);
double getRandom(double,double);

int main(int argc, const char* argv[])
{
	// init GPIO
	bcm2835_init();

#ifdef _CONNECTED
	// init pololu
#ifdef _POLOLU_CONNECTED
	int retourInitPololu = initPololu("/dev/ttyS0");
	if(retourInitPololu < 0)
	{
		perror("Error while initializing pololu");
		exit(1);
	}
#endif // _POLOLU_CONNECTED

#ifdef _ADC_CONNECTED
	// init ADC
	int retourInitADC = initADS1115();
	if(retourInitADC < 0)
	{
		perror("Error while initializing ADC");
		exit(1);
	}
#endif //_ADC_CONNECTED

#ifdef _TEMP_CONNECTED
	// init Temperature
	int retourInitTemp = initMAX6675();
	if(retourInitTemp < 0)
	{
		perror("Error while initializing MAX6675");
		exit(1);
	}
#endif //_TEMP_CONNECTED

#ifdef _SCALE_CONNECTED
	// init scale (voir les pin)
	hx711Sruct = (HX711*)malloc(sizeof(HX711));
	int retourInitScale = initHX711(hx711Sruct, 20, 21);
	if(retourInitScale)
	{
		perror("Error while initializing HX711 scale");
		exit(1);
	}

	setupGPIO(hx711Sruct);
	reset(hx711Sruct);
#endif //_SCALE_CONNECTED

	//zeroScale(hx711Sruct);
#endif //_CONNECTED

	const char* serverName = "math-main-pi.local";

	socketFdCmd = initClient(serverName, COMMAND_PORT);
	while(socketFdCmd < 0)
	{
		sleep(1);
		socketFdCmd = initClient(serverName, COMMAND_PORT);
	}

	socketFdSts = initClient(serverName, STATUS_PORT);
	while(socketFdSts < 0)
	{
		sleep(1);
		socketFdSts = initClient(serverName, STATUS_PORT);
	}

	printf("connexion établis \n");

	// Send message to discover client
	char *data = "RPI-Control";

	sendMessage(socketFdCmd, data, strlen(data), ID_MSG, RPI_CONTROL_ID);
	sendMessage(socketFdSts, data, strlen(data), ID_MSG, RPI_CONTROL_ID);

	printf("message envoyé \n");

	// init fsm status struct
	initMainFsm();

	// create and start threads
	pthread_attr_t attribute;
	pthread_attr_init(&attribute);

	// create command thread
	pthread_create(&commandThread, &attribute, commandThreadFunction,&socketFdCmd);
	pthread_create(&statusThread, &attribute, statusThreadFunction, &socketFdSts);
	pthread_create(&tempThread, &attribute, asservissementTemperatureThreadFunction, NULL);
	pthread_create(&fsmThread, &attribute, FSMThreadFunction, NULL);

	// join and start
	pthread_join(commandThread, NULL);
	pthread_join(statusThread, NULL);
	pthread_join(statusThread, NULL);
	pthread_join(fsmThread, NULL);

	printf("application terminée ! \n");
	return 0;
}

/*
 * reading status and sending to user interface
 */
void *statusThreadFunction(void* param)
{
	// init
	int socketFd = *((int*)(param));
	double random_value = 0;
	int count = 0;
	int sendError;
	char valueToSend[255];

	printf("allo je suis le thread de status \n");

	while(1)
	{
#ifdef _CONNECTED
		// read crusher current
#ifdef _ADC_CONNECTED
		double voltage = readChannelValueADS1115(CRUSHER_MOTOR_CURRENT_CHANNEL);
		double current = voltageToCurrentCrusher(voltage);
		printf("Courant déchiqueteur : %f \n", current);
		mainFsmStatus->crusherCurrent = current;

		// read extruder current
		voltage = readChannelValueADS1115(EXTRUDER_MOTOR_CURRENT_CHANNEL);
		current = voltageToCurrentExtruder(voltage);
		printf("Courant extrudeur : %f \n", current);
		mainFsmStatus->extruderCurrent = current;
#endif //_ADC_CONNECTED

#ifdef _TEMP_CONNECTED
		// read and send temerature
		double temperature = readTemperatureMAX6675();
		mainFsmStatus->extruderTemperature = temperature;
		sprintf(valueToSend, "%.5g", temperature);
		printf("Température : %f\n", temperature);
		sendMessage(socketFd,(char*)( &valueToSend), strlen(valueToSend), STS_TEMPERATURE_MSG, RPI_CONTROL_ID);
#endif


#ifdef _SCALE_CONNECTED
		// read weight and send
		unsigned int weight = getRawData(hx711Sruct);
		double poidsG = voltageToWeight(weight);
		sprintf(valueToSend, "%.5g", poidsG);
		printf("Poids count: %u \n", weight);
		printf("Poids  grams : %f \n", poidsG);
		mainFsmStatus->convoyWeight = poidsG;
		sendMessage(socketFd,(char*)( &valueToSend), strlen(valueToSend), STS_WEIGHT_MSG, RPI_CONTROL_ID);
#endif //_SCALE_CONNECTED

		// send status to interface
		if(mainFsmStatus->fsmRequestedMode == FSM_EXTRUDER_MODE)
		{
			if(strlen(mainFsmStatus->fsmExtruderStatus) != 0)
			{
				sendMessage(socketFd, mainFsmStatus->fsmExtruderStatus, strlen(mainFsmStatus->fsmExtruderStatus), STS_FSM_MSG, RPI_CONTROL_ID);
			}

		}else
		{
			if(strlen(mainFsmStatus->fsmCrusherStatus) != 0)
			{
				sendMessage(socketFd, mainFsmStatus->fsmCrusherStatus, strlen(mainFsmStatus->fsmCrusherStatus), STS_FSM_MSG, RPI_CONTROL_ID);
			}
		}



		if(mainFsmStatus->crusherIsBlocked)
		{
			// le déchiquteur est bloqué
			sendMessage(socketFd, "true", strlen("true"), STS_CRUSHER_BLOCKED_MSG, RPI_CONTROL_ID);
		}else
		{
			// le déchiquteur est bloqué
			sendMessage(socketFd, "false", strlen("false"), STS_CRUSHER_BLOCKED_MSG, RPI_CONTROL_ID);
		}

		/******************** FIN VRAI CODE À TESTER POUR LECTURE DES COMPOSANTS ****************************/
#endif //_CONNECTED

#ifdef _DEBUG
		// code de test d'affichage des données
		// send random temeprature status
		random_value = getRandom(0,200);
		sprintf(valueToSend, "%.2g", random_value);
		mainFsmStatus->extruderTemperature = random_value;
		sendError = sendMessage(socketFd,(char*)( &valueToSend), strlen(valueToSend), STS_TEMPERATURE_MSG, RPI_CONTROL_ID);

		//send random weight status
		random_value = getRandom(0,2000);
		sprintf(valueToSend, "%.2g", random_value);
		mainFsmStatus->convoyWeight = random_value;
		sendError = sendMessage(socketFd, (char*)( &valueToSend), strlen(valueToSend), STS_WEIGHT_MSG, RPI_CONTROL_ID);


		// send FSM status
		if(count % 2 == 0)
		{
			sendError = sendMessage(socketFd, "Chauffage extrudeur", strlen("Chauffage extrudeur"), STS_FSM_MSG, RPI_CONTROL_ID);
			memcpy(mainFsmStatus->fsmExtruderStatus, "Chauffage extrudeur", strlen("Chauffage extrudeur"));
		}else
		{
			sendError = sendMessage(socketFd, "Mesure du poids", strlen("Mesure du poids"), STS_FSM_MSG, RPI_CONTROL_ID);
			memcpy(mainFsmStatus->fsmExtruderStatus, "Mesure du poids", strlen("Mesure du poids"));
		}
		// send blocked or not
		if(count >= 10000)
		{
			sendError = sendMessage(socketFd, "false", strlen("false"), STS_CRUSHER_BLOCKED_MSG, RPI_CONTROL_ID);
			mainFsmStatus->crusherIsBlocked = 0;
		}else
		{
			sendError = sendMessage(socketFd, "true", strlen("true"), STS_CRUSHER_BLOCKED_MSG, RPI_CONTROL_ID);
			mainFsmStatus->crusherIsBlocked = 1;
		}

		count++;
#endif //_DEBUG

		if(terminated == 1)break;

		sleep(1);//usleep(500000);
	}

	// free and exit
	free(hx711Sruct);
	pthread_exit(NULL);

}

/*
 * Command commign from user interface thread management
 */
void *commandThreadFunction(void* param)
{
	int socketFd = *((int*)(param));
	printf("allo je suis le thread de commande \n");
	// initialize command
	RPIMessageHeader_t *header = (RPIMessageHeader_t*)malloc(sizeof(RPIMessageHeader_t));
	char *message = (char*)malloc(255);
	int exit = -1;

	// read command
	while(1)
	{
		receiveMessage(socketFd, header, message);

		switch(header->PayloadType)
		{
			case MANUAL_CMD_SERVO:
				// set motor to position
				if(((t_pololu_cmd*)message)->channel == 1)
				{
					int value = ((t_pololu_cmd*)message)->value;
					double dutyCycle = ((double)value)/((double)2000.0);
					setPWM(dutyCycle);
					printf("J'ai mis le pwm \n");
				}else
				{
					setPosition(((t_pololu_cmd*)message)->channel, ((t_pololu_cmd*)message)->value);
					printf("allo \n");
				}

				exit = ((t_pololu_cmd*)message)->channel;

				break;

			case CRUSHER_MODE_CMD:
				// get weight
				mainFsmStatus->requestedWeight = atof(message);
				printf("Poids demandé : %f\n", mainFsmStatus->requestedWeight);
				mainFsmStatus->fsmRequestedMode = FSM_CRUSHER_MODE;
				// start FSM
				startCrusherFSM();


				// verify that extuder mode is not requested
				// if same mode is requested, continue
				// if other mode is requested, send error, we have to stop before chaging mode
				break;

			case EXTRUDER_MODE_CMD:
				//begin extruder mode
				mainFsmStatus->requestedTemperature = atof(message);
				startExtruderFSM();
				// verify that CRUSHER mode is not requested
				// if same mode is requested, continue
				// if other mode is requested, send error, we have to stop before chaging mode
				break;

			case HEATER_MANUAL_CMD:
				// begin asservissement of heater
				mainFsmStatus->requestedTemperature = atof(message);
				setTemperatureCommand();

				// start enslavement
				mainFsmStatus->enslavementStarted = 1;
				break;

			case STOP_CMD:
				// stop all (user requested)
				stopCrusherFSM();
				stopExtruderFSM();
				mainFsmStatus->enslavementStarted = 0;
				mainFsmStatus->requestedTemperature = 0.0;
				mainFsmStatus->requestedWeight = 0.0;
				break;

			default:
				break;
		}

		if(exit == 4) break;
	}

	terminated = 1;

	pthread_exit(NULL);
}

void *asservissementTemperatureThreadFunction(void* param)
{
	double commandToApply = 0.0;
	printf("Je suis le thread de l'asservissement \n");
	while(1)
	{
		if(mainFsmStatus->enslavementStarted == 1)
		{
			// apply enslavement on temperature
			printf("Requested temp : %f \n", mainFsmStatus->requestedTemperature);
			printf("Actual temp : %f \n", mainFsmStatus->extruderTemperature);
			commandToApply = calculateActionToApply(mainFsmStatus->extruderTemperature); // duty cycle
			printf("Voltage to apply : %f\n", commandToApply);
			commandToApply = voltageToDutyCycle(commandToApply);

			printf("Command to apply : %f \n", commandToApply);

			// apply command to extruder
			setPWM(commandToApply);

		}else
		{
			clearTemperatureCommand();
			setPWM(0);
		}

		// verifiy if we want to exit
		if(terminated == 1) break;

		sleep(1);

	}

	pthread_exit(NULL);
}

void *FSMThreadFunction(void* param)
{
	printf("Allo je suis le thread de FSM\n");
	while(1)
	{
		// update FSM
		updateCrusherFSM();
		updateExtruderFSM();

		// verifiy if we want to exit
		if(terminated == 1) break;

		usleep(500000);
	}
}

double getRandom(double min,double max)
{
	srand(time(NULL));
	double random_value ;
	double range = max - min;
	double div = RAND_MAX / range;

	// generate random value
	return random_value = min + (rand()/div);
}


