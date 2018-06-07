/******************************************************************************/
/*                                                                            */
/* motord.c - motor daemon for controlling boat motors connected to the       */
/*            Raspberry Pi.                                                   */
/*                                                                            */
/*            Creates and listens to requests on a message queue to control   */
/*            the motor. The motor interface is defined in motor.h            */
/*                                                                            */
/*            The daemon also creates a listener and sender thread for an     */
/*            external connection.                                            */
/*                                                                            */
/******************************************************************************/

// Standard headers
#include <stdlib.h>
#include <stdio.h>
#include <dlfcn.h>
#include <stdarg.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <errno.h>

// For message queue
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

// For signal handling
#include <signal.h>

// For posix threads
#include <pthread.h>

// For rand48
#include <sys/time.h>

// Socket headers
#include <sys/socket.h> // Socket structure declarations
#include <netinet/in.h> // Structures needed for internet domain addresses
#include <netdb.h>      // Defines structure hostnet

#include "motor.h"


#define M_ID    2013

// PID of forked wiimote listener
int wpid;

// message queue variable
// This is global because it will be read by multiple threads.
int msqid;

// These threads are responsible for keeping lifetime connection to other nodes.
// This will receive messages from message queue and send them out the socket.
void *sender_thread(void *arg);
// This will listen to the socket and place messages on message queue.
void *receiver_thread(void *arg);

// Signal thread
// This will remain blocked until a signal arrives 
// (recommended technique when dealing with threads and signals)
void *sig_waiter(void *arg);
// Signal handler function
void sig_handler(int sig);

// Error function to exit gracefully (frees resources)
void error(int error_code, char *msg, ...) { 
	va_list argptr;
	
	// Error code is used to figure out if there are resources that need to be freed.
	switch (error_code) {
		case 2:
			// Listener/Receiver thread cleanup 
			// TODO clean up threads 
		case 1:
			// Message queue cleanup
			// Deleting message queue
			msgctl(msqid, IPC_RMID, NULL);
		default:
			break;
	}

	// Get variable number of arguments and pass to fprintf.
	va_start(argptr, msg);
	vfprintf(stderr, msg, argptr);
	va_end(argptr);	

	exit(error_code);
}

int main(int argc, char *argv[]) { 
	// Command line arguments
	// General variables
	int i;
	char buffer[256];

	// Socket variables
	int sockfd_c, sockfd_l, newsockfd;
	struct sockaddr_in serv_addr, host_addr;
	struct hostent *server;

	// Signal handler variables
	sigset_t all_signals;
	struct sigaction new_act;
	int nsigs;
	int sigs[] = { SIGINT, SIGTERM, SIGBUS, SIGSEGV, SIGFPE };

	// Thread variables
	pthread_t handlr_id, snd_id, rcv_id;

	// Setup for managing signals	
	sigfillset(&all_signals);
	nsigs = sizeof(sigs) / sizeof(int);
	for ( i = 0; i < nsigs; i++ )
		sigdelset(&all_signals, sigs[i]);
	// Blocking all signals other than those listed in the signal array above.
	sigprocmask(SIG_BLOCK, &all_signals, NULL);
	sigfillset(&all_signals);
	for ( i = 0; i < nsigs; i++ ) {
		new_act.sa_handler = sig_handler;
		new_act.sa_mask    = all_signals;
		new_act.sa_flags   = 0;
		// This is saying that if the listed signals are receive, 
		// ignore any other incoming signals and run the sig_handler.
		if (sigaction(sigs[i], &new_act, NULL) == -1) {
			perror("Can't set signals :\n");
			exit(1);
		}	
	}

	// Creating message queue.
	// This will be used to relay messages from:
	// 1. Local processes (bluetooth remote) that wants to send message to handler
	// 2. The receiver thread to the message handler thread
	// 3. The handler thread to sender thread
	if ((msqid = msgget(M_ID, (IPC_CREAT | 0666))) == -1) {
		perror("msgget failed :\n");
		exit(1);
	}
	
	// Make message handler thread to be defined in motor.c
	if (pthread_create(&handlr_id, NULL, msg_handler, (void *) &msqid) != 0) {
		fprintf(stderr, "pthread_create failed\n");
		exit(1);
	}

	// Fork bluetooth wiimote process
	wpid = fork();
	switch( wpid ) {
		case -1:
			error(2, "Error forking");
		case  0:
			execl("./wiimoted.py", NULL);
			perror("Error running bluetooth listener\n");
			exit(1);
	}

	// Setting up socket
	if ((sockfd_l = socket(AF_INET, SOCK_STREAM, 0)) == -1)
		error(1, "Error on socket creation\n");

	// Set up host address
	bzero((char*) &host_addr, sizeof(host_addr));
	host_addr.sin_family      = AF_INET;
	host_addr.sin_addr.s_addr = INADDR_ANY;
	host_addr.sin_port        = htons(NC_PORT);

	if (bind(sockfd_l, (struct sockaddr *) &host_addr, sizeof(host_addr)) < 0)
		error(2, "Error on bind\n");
	
	// Setting socket to listen mode, with a backlog queue of five (max).
	listen(sockfd_l, 5);

	snd_id = 0;
	rcv_id = 0;
	while(1) {

		if ((newsockfd = accept(sockfd_l, NULL, NULL)) < 0)
			error(2, "Error on accept.\n");
			
		printf("new connection made\n");
		
		if (snd_id != 0 && rcv_id != 0) {
			pthread_cancel(snd_id);
			pthread_join(snd_id, NULL);
		}
		if (rcv_id != 0) {
			pthread_cancel(rcv_id);
			pthread_join(rcv_id, NULL);
		}

		// Start receiver thread.
		if (pthread_create(&snd_id, NULL, receiver_thread, (void *) &newsockfd) != 0) {
			perror("Error on thread creation\n");
			exit(1);
		}	
		
		// Start sender thread.
		if (pthread_create(&rcv_id, NULL, sender_thread, (void *) &newsockfd) != 0) {
			perror("Error on thread creation\n");
			exit(1);
		}
		
	}

	// Wait for message handler to exit (should never happen)
	pthread_join(handlr_id, NULL);

	// Deleting message queue
	if (msgctl(msqid, IPC_RMID, NULL) == -1) {
		fprintf(stderr, "Message queue could not be deleted.\n");
		exit(1);
	}

	return 0;
}

void sig_handler(int sig) {
	if( kill(wpid, SIGKILL) != 0)
		perror("Error killing wiimote listener process");

	// Deleting message queue
	if (msgctl(msqid, IPC_RMID, NULL) == -1) {
		fprintf(stderr, "Message queue could not be deleted.\n");
		exit(1);
	}
	exit(0);
}

void *sig_waiter(void *arg) {
	while (1)
		pause();
}

void *receiver_thread(void *arg) {
	// Receiver gets message from socket and places it in the message queue. 
	int sockfd = *((int *) arg);
	int i;
	MSG inmsg;

	for (;;) {
		// Read messages from socket.
		// Each message has a header that tells the number of bytes in the actual message.
		if ((i = read(sockfd, &inmsg.msg, 1)) == -1) {
			perror("Error on socket read");
			return NULL;
		}

		if (i == 0)
			return NULL;
		
		// Place message on message queue for handler to process.
		inmsg.mtype = TO_HNDLR;

		if (msgsnd(msqid, &inmsg, sizeof(MSG)-sizeof(long), 0) == -1) {
			perror("Error in message queue");
			sig_handler(-1);
		}
	}
	return NULL;
}

void *sender_thread(void *arg) {
	// Sender thread listens to the message queue, and then writes its message to socket.
	int sockfd = *((int *) arg);
	MSG omsg;

	for (;;) {
		// Blocks until a message for sending is received from the queue
		// First a message stating what type of process is going to send a request is received,
		// Then the actual message will be given. 
		if (msgrcv(msqid, &omsg, sizeof(MSG), TO_SNDR, 0) == -1) {
			perror("Error on message queue receive");
			return NULL;
		}


		if (write(sockfd, &omsg.msg, 1) == -1) {
			perror("Error on socket write");
			return NULL;
		}

		if (omsg.msg == DISCONNECT)
			return NULL;
	}
	return NULL;
}
