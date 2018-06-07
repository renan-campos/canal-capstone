#include <stdio.h>
#include <stdlib.h>
#include <sys/msg.h>
#include <unistd.h>
#include <pigpio.h>


#define HBRIDGE_R_F 17
#define HBRIDGE_R_B 27
 
#define HBRIDGE_L_F 23
#define HBRIDGE_L_B 24
// NOTE: DONT EVER TURN BOTH FORWARD AND BACK AT THE SAME TIME!

#include "motor.h"

void *msg_handler(void *arg) {
	int msqid = *((int *) arg);
	MSG m, omsg;
	char remote = 0; 
	char state = STOPPED;

	printf("Message handler started\n");
	fflush(stdout);

         if (gpioInitialise() < 0) return 0;
 
 	gpioSetMode(HBRIDGE_L_F, PI_OUTPUT);
 	gpioSetMode(HBRIDGE_L_B, PI_OUTPUT);
 	gpioSetMode(HBRIDGE_R_F, PI_OUTPUT);
 	gpioSetMode(HBRIDGE_R_B, PI_OUTPUT);
 	
 	gpioWrite(HBRIDGE_L_F, 0);
 	gpioWrite(HBRIDGE_L_B, 0);
 	gpioWrite(HBRIDGE_R_F, 0);
 	gpioWrite(HBRIDGE_R_B, 0);

	for (;;) {
		if (msgrcv(msqid, &m, sizeof(MSG), TO_HNDLR, 0) == -1) {
			perror("Handler");
			return NULL;
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
					return NULL;
				}
				continue;
			case CONNECT:
				printf("AI CONNECT received.\n");
				omsg.mtype = TO_SNDR;
				omsg.msg   = SUCCESS;
				if (msgsnd(msqid, &omsg, sizeof(MSG)-sizeof(long), 0) == -1) {
					perror("Error in message queue");
					return NULL;
				}
				continue;
			case DISCONNECT:
				printf("AI DISCONNECT received.\n");
				omsg.mtype = TO_SNDR;
				omsg.msg   = DISCONNECT;
				if (msgsnd(msqid, &omsg, sizeof(MSG)-sizeof(long), 0) == -1) {
					perror("Error in message queue");
					return NULL;
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
					return NULL;
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
 			
 		        gpioHardwarePWM(18, 500, 990000);
 			
 			gpioWrite(HBRIDGE_L_F, 0);
 			gpioWrite(HBRIDGE_L_B, 0);
 			gpioWrite(HBRIDGE_R_F, 0);
 			gpioWrite(HBRIDGE_R_B, 0);
 
 			// Sleep 1 seconds to make sure motor current dissapated.
 			sleep(1);
			if (!remote) {
				omsg.mtype = TO_SNDR;
				omsg.msg   = SUCCESS;
				if (msgsnd(msqid, &omsg, sizeof(MSG)-sizeof(long), 0) == -1) {
					perror("Error in message queue");
					return NULL;
				}
			}
			break;
		case FORWARD:
			state = FORWARD;
			printf("FORWARD received.\n");
 		        
 			gpioHardwarePWM(18, 500, 250000);
 			
 			gpioWrite(HBRIDGE_L_F, 1);
 			gpioWrite(HBRIDGE_L_B, 0);
 			gpioWrite(HBRIDGE_R_F, 1);
 			gpioWrite(HBRIDGE_R_B, 0);
			if (!remote) {
				omsg.mtype = TO_SNDR;
				omsg.msg   = SUCCESS;
				if (msgsnd(msqid, &omsg, sizeof(MSG)-sizeof(long), 0) == -1) {
					perror("Error in message queue");
					return NULL;
				}
			}
			break;
		case BACK:
			state = BACK;
			printf("BACK received.\n");
 		        
 			gpioHardwarePWM(18, 500, 990000);
 
 			gpioWrite(HBRIDGE_L_F, 0);
 			gpioWrite(HBRIDGE_L_B, 1);
 			gpioWrite(HBRIDGE_R_F, 0);
 			gpioWrite(HBRIDGE_R_B, 1);
			if (!remote) {
				omsg.mtype = TO_SNDR;
				omsg.msg   = SUCCESS;
				if (msgsnd(msqid, &omsg, sizeof(MSG)-sizeof(long), 0) == -1) {
					perror("Error in message queue");
					return NULL;
				}
			}
			break;
		case RIGHT:
			state = RIGHT;
			printf("RIGHT received.\n");
 			
 			gpioHardwarePWM(18, 500, 250000);
 
 			gpioWrite(HBRIDGE_L_F, 0);
 			gpioWrite(HBRIDGE_L_B, 0);
 			gpioWrite(HBRIDGE_R_F, 1);
 			gpioWrite(HBRIDGE_R_B, 0);
			if (!remote) {
				omsg.mtype = TO_SNDR;
				omsg.msg   = SUCCESS;
				if (msgsnd(msqid, &omsg, sizeof(MSG)-sizeof(long), 0) == -1) {
					perror("Error in message queue");
					return NULL;
				}
			}
			break;
		case LEFT:
			state = LEFT;
			printf("LEFT received.\n");
 			
 			gpioHardwarePWM(18, 500, 250000);
 
 			gpioWrite(HBRIDGE_L_F, 1);
 			gpioWrite(HBRIDGE_L_B, 0);
 			gpioWrite(HBRIDGE_R_F, 0);
 			gpioWrite(HBRIDGE_R_B, 0);
			if (!remote) {
				omsg.mtype = TO_SNDR;
				omsg.msg   = SUCCESS;
				if (msgsnd(msqid, &omsg, sizeof(MSG)-sizeof(long), 0) == -1) {
					perror("Error in message queue");
					return NULL;
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
 	gpioTerminate();
	return NULL;
}
