/* An interactive cli to serve as an example for the motor api */
#include "motor_api.h"

#include <stdio.h>

int main(int argc, char *argv[]) {
	char c, r;
	MOTOR motor;

	if (argc < 2) {
		fprintf(stderr, "Expected IP of node running motord as command line argument.\n");
		return 1;
	}
	
	printf("motor api example\n"
			"c : connect\n"
			"d : disconnect\n"
			"f : forward\n"
			"b : back\n"
			"r : right\n"
			"l : left\n"
			"s : state\n"
			"q : quit\n");

	
	printf("Enter cmd: ");
	while(1) {
		scanf("%c", &c);
		switch(c) {
			case 'c':
				printf("Connecting...\n");
				r = motor_connect(&motor, argv[1]);
				printf("motord returned: %d\n", r);
				break;
			case 'd':
				printf("Disconnecting...\n");
				r = motor_disconnect(&motor);
				printf("motord returned: %d\n", r);
				motor = -1;
				break;
			case 'f':
				printf("Forward...\n");
				r = motor_forward(motor);
				printf("motord returned: %d\n", r);
				break;
			case 'b':
				printf("Back...\n");
				r = motor_back(motor);
				printf("motord returned: %d\n", r);
				break;
			case 'l':
				printf("Left...\n");
				r = motor_left(motor);
				printf("motord returned: %d\n", r);
				break;
			case 'r':
				printf("Right...\n");
				r = motor_right(motor);
				printf("motord returned: %d\n", r);
				break;
			case 's':
				printf("State...\n");
				r = motor_state(motor);
				printf("motord returned: %d\n", r);
				break;
			case 'q':
				if (motor != -1)
					motor_disconnect(&motor);
				return 0;
			case '\n':
				printf("Enter cmd: ");

		}
	}
	return 0;
}
