/*
 * communication.c
 *
 *  Created on: Feb. 18, 2019
 *      Author: mathieu
 */

#include "communication.h"

/*
 * Send message with header
 * return number of bytes writenn
 * on error, return -1 when size < 0 and retunr -2 if error while writing
 */
int sendMessage(int socketFd, char* ptrSend, int sendSize,  int type, int source)
{
	// create header to send
	RPIMessageHeader_t *header = (RPIMessageHeader_t*)malloc(sizeof(RPIMessageHeader_t));
	memset(&(header->Signature), 0, 4);
	memcpy(&(header->Signature), "RPI\0", 4);
	header->PayloadSize = sendSize;
	header->PayloadSource = source;
	header->PayloadType = type;

	if(sendSize < 0)
	{
		free(header);
		return -1;
	}

	int nbHeaderSend, nbDataSend = 0;
	// send header
	nbHeaderSend =  write(socketFd, header, sizeof(RPIMessageHeader_t));

	if(nbHeaderSend < 0)
	{
		free(header);
		return -2;
	}

	// send data
	nbDataSend = write(socketFd, ptrSend, sendSize);

	if(nbDataSend < 0)
	{
		free(header);
		return -2;
	}

	free(header);

	return  nbHeaderSend + nbDataSend;
}

/*
 * receive message with header (blocking)
 * return number of bytes received
 * on error, return -1 if error while reading
 */
int receiveMessage(int socketFd, RPIMessageHeader_t* ptrHeader, char* ptrRead)
{
	int nbHeaderRcv, nbDataRcv = 0;

	// read header from socket
	nbHeaderRcv = read(socketFd, ptrHeader, sizeof(RPIMessageHeader_t));

	if(nbHeaderRcv < 0)
	{
		return -1;
	}

	// read rest of message
	while(nbDataRcv != ptrHeader->PayloadSize)
	{
		nbDataRcv += read(socketFd, ptrRead + nbDataRcv, ptrHeader->PayloadSize - nbDataRcv);
	}

	if(nbDataRcv)
	{
		return -1;
	}

	return nbHeaderRcv + nbDataRcv;
}


