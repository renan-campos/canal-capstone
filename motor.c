#include <stdio.h>
#include <stdlib.h>
#include <sys/msg.h>

#include "motor.h"

void *msg_handler(void *arg) {
	int msqid = *((int *) arg);
	MSG m, omsg;
	char remote = 0; 
	char state = STOPPED;

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
			switch(m.msg) {
			case STATE:
				printf("STATE received.\n");
				omsg.mtype = TO_SNDR;
				omsg.msg   = state;
				if (msgsnd(msqid, &omsg, sizeof(MSG)-sizeof(long), 0) == -1) {
					perror("Error in message queue");
					return;
				}
				continue;
			case CONNECT:
				printf("AI CONNECT received.\n");
				omsg.mtype = TO_SNDR;
				omsg.msg   = SUCCESS;
				if (msgsnd(msqid, &omsg, sizeof(MSG)-sizeof(long), 0) == -1) {
					perror("Error in message queue");
					return;
				}
				continue;
			case DISCONNECT:
				printf("AI DISCONNECT received.\n");
				omsg.mtype = TO_SNDR;
				omsg.msg   = DISCONNECT;
				if (msgsnd(msqid, &omsg, sizeof(MSG)-sizeof(long), 0) == -1) {
					perror("Error in message queue");
					return;
				}
				continue;
			}
			// Ignore message from sender (other than STATE) if remote is on
			if (remote) {
				printf("REMOTE connnected %d\n", remote);
				omsg.mtype = TO_SNDR;
				omsg.msg   = CONNECT;
				if (msgsnd(msqid, &omsg, sizeof(MSG)-sizeof(long), 0) == -1) {
					perror("Error in message queue");
					return;
				}
				continue;
			}
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
			state = STOPPED;
			printf("STOPPED received.\n");
			if (!remote) {
				omsg.mtype = TO_SNDR;
				omsg.msg   = SUCCESS;
				if (msgsnd(msqid, &omsg, sizeof(MSG)-sizeof(long), 0) == -1) {
					perror("Error in message queue");
					return;
				}
			}
			break;
		case FORWARD:
			state = FORWARD;
			printf("FORWARD received.\n");
			if (!remote) {
				omsg.mtype = TO_SNDR;
				omsg.msg   = SUCCESS;
				if (msgsnd(msqid, &omsg, sizeof(MSG)-sizeof(long), 0) == -1) {
					perror("Error in message queue");
					return;
				}
			}
			break;
		case BACK:
			state = BACK;
			printf("BACK received.\n");
			if (!remote) {
				omsg.mtype = TO_SNDR;
				omsg.msg   = SUCCESS;
				if (msgsnd(msqid, &omsg, sizeof(MSG)-sizeof(long), 0) == -1) {
					perror("Error in message queue");
					return;
				}
			}
			break;
		case RIGHT:
			state = RIGHT;
			printf("RIGHT received.\n");
			if (!remote) {
				omsg.mtype = TO_SNDR;
				omsg.msg   = SUCCESS;
				if (msgsnd(msqid, &omsg, sizeof(MSG)-sizeof(long), 0) == -1) {
					perror("Error in message queue");
					return;
				}
			}
			break;
		case LEFT:
			state = LEFT;
			printf("LEFT received.\n");
			if (!remote) {
				omsg.mtype = TO_SNDR;
				omsg.msg   = SUCCESS;
				if (msgsnd(msqid, &omsg, sizeof(MSG)-sizeof(long), 0) == -1) {
					perror("Error in message queue");
					return;
				}
			}
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
