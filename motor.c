#include <stdio.h>
#include <stdlib.h>
#include <sys/msg.h>

#include "motor.h"

void *msg_handler(void *arg) {
	int msqid = *((int *) arg);
	MSG m;
	char remote = 0; 

	printf("Message handler started\n");
	fflush(stdout);

	for (;;) {
		if (msgrcv(msqid, &m, sizeof(MSG), TO_HNDLR, 0) == -1) {
			perror("Handler");
			return;
		}

		printf("Received message %d\n", (int) m.msg);
		fflush(stdout);

		// Determine who sent it.
		if (m.msg >= 20) {
		// Sent from sender
			printf("Message from auto-pilot\n");
			m.msg -= 20;
			// Ignore message from sender (other than STATE) if remote is on
			if (remote && m.msg != STATE)
				continue;
		}
		else if (m.msg >= 10) {
		// Sent from remote 
			printf("Message from remote control\n");
			m.msg -= 10;
		}
		else {
			printf("Invalid message\n");
			continue;
		}	

		switch(m.msg) {
		case STOPPED:
			printf("STOPPED received.\n");
			break;
		case FORWARD:
			printf("FORWARD received.\n");
			break;
		case BACK:
			printf("BACK received.\n");
			break;
		case RIGHT:
			printf("RIGHT received.\n");
			break;
		case LEFT:
			printf("LEFT received.\n");
			break;
		case STATE:
			printf("STATE received.\n");
			break;
		case CONNECT:
			printf("CONNECT received.\n");
			remote = 1;
			break;
		case DISCONNECT:
			printf("DISCONNECT received.\n");
			remote = 0;
			break;
		}
	}
	return NULL;
}
