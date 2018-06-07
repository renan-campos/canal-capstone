#include "motor_api.h"
#include "motor.h"

#include <stdlib.h>
#include <stdio.h>
#include <strings.h>
#include <unistd.h>


char motor_connect(MOTOR *motor, char *addr) {
	int n;
	struct hostent *server;	
	struct sockaddr_in serv_addr;

	// Creating new socket for connection
	if ((*motor = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("Error on socket creation");
		return FAIL;
	}
	
	// Getting host information in order to connect.
	server = gethostbyname(addr);

	if (server == NULL) {
		fprintf(stderr, "Failed on hostname resolution\n");
		return FAIL;
	}

	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	bcopy((char *) server->h_addr, (char *) &serv_addr.sin_addr.s_addr, server->h_length);
	serv_addr.sin_port = htons(NC_PORT);

	if (connect(*motor, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
		perror("Failed on motor connection");
		return FAIL;
	}
	
	n = CONNECT+FROM_RCVR;
	// Once the connection has been made, provide a node id to the connected server, and read its node id.
	if (write(*motor, &n, 1) != 1) {
		perror("Failed writing to motor");
		return FAIL;
	}
	if (read(*motor, &n, 1) != 1) {
		perror("Failed reading from motor");
		return FAIL;
	}

	return n;
}

char motor_stop(MOTOR motor) {
	int n;
	n = STOPPED+FROM_RCVR;
	if (write(motor, &n, 1) != 1) {
		perror("Failed writing to motor");
		return FAIL;
	}
	if (read(motor, &n, 1) != 1) {
		perror("Failed reading from motor");
		return FAIL;
	}

	return n;
}

char motor_forward(MOTOR motor) {
	int n;
	n = FORWARD+FROM_RCVR;
	if (write(motor, &n, 1) != 1) {
		perror("Failed writing to motor");
		return FAIL;
	}
	if (read(motor, &n, 1) != 1) {
		perror("Failed reading from motor");
		return FAIL;
	}

	return n;
}

char motor_back(MOTOR motor) {
	int n;
	n = BACK+FROM_RCVR;
	if (write(motor, &n, 1) != 1) {
		perror("Failed writing to motor");
		return FAIL;
	}
	if (read(motor, &n, 1) != 1) {
		perror("Failed reading from motor");
		return FAIL;
	}

	return n;
}

char motor_right(MOTOR motor) {
	int n;
	n = RIGHT+FROM_RCVR;
	if (write(motor, &n, 1) != 1) {
		perror("Failed writing to motor");
		return FAIL;
	}
	if (read(motor, &n, 1) != 1) {
		perror("Failed reading from motor");
		return FAIL;
	}

	return n;
}

char motor_left(MOTOR motor) {
	int n;
	n = LEFT+FROM_RCVR;
	if (write(motor, &n, 1) != 1) {
		perror("Failed writing to motor");
		return FAIL;
	}
	if (read(motor, &n, 1) != 1) {
		perror("Failed reading from motor");
		return FAIL;
	}

	return n;
}

char motor_state(MOTOR motor) {
	int n;
	n = STATE+FROM_RCVR;
	if (write(motor, &n, 1) != 1) {
		perror("Failed writing to motor");
		return FAIL;
	}
	if (read(motor, &n, 1) != 1) {
		perror("Failed reading from motor");
		return FAIL;
	}

	return n;
}

char motor_disconnect(MOTOR *motor) {
	int n;
	n = DISCONNECT+FROM_RCVR;
	if (write(*motor, &n, 1) != 1) {
		perror("Failed writing to motor");
		return FAIL;
	}
	if (read(*motor, &n, 1) != 1) {
		perror("Failed reading from motor");
		return FAIL;
	}

	close(*motor);

	return n;
}
