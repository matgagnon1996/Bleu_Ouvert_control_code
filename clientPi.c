/*
 * clientPi.c
 *
 *  Created on: Feb. 16, 2019
 *      Author: mathieu
 */

#include "clientPi.h"
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int socketFd = 0;


int initClient(const char* serverName, int portNumber)
{
	// init //
	struct sockaddr_in serverAddr;
	struct hostent *server;

	// create socket //
	socketFd = socket(AF_INET, SOCK_STREAM, 0);

	if(socketFd < 0)
	{
		perror("Error while openning socket");
		return socketFd;
	}

	// get server //
	server = gethostbyname(serverName);

	if(!server)
	{
		perror("No such Host");
		return -1;
	}

	// operate on server address //
	bzero((char *) &serverAddr, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	bcopy((char *)server->h_addr, (char *)&serverAddr.sin_addr.s_addr, server->h_length);
	serverAddr.sin_port = htons(portNumber);

	int errorConnect = connect(socketFd, (struct sockaddr*)&serverAddr, sizeof(serverAddr));

	if(errorConnect < 0)
	{
		perror("Error while connecting");
		return errorConnect;
	}

	return socketFd;
}


