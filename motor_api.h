#ifndef _motorapi
#define _motorapi


// Socket headers
#include <sys/socket.h> // Socket structure declarations
#include <netinet/in.h> // Structures needed for internet domain addresses
#include <netdb.h>      // Defines structure hostnet

/* Defines API for autonomous program to use to control the motor. */

typedef int MOTOR;

// Creates a websocket connection to motor daemon. 
// Returns SUCCESS or FAIL
char motor_connect(MOTOR *motor, char *addr);

// The following return SUCCESS, FAIL, or CONNECT if wiimote connected.
char motor_stop(MOTOR motor);
char motor_forward(MOTOR motor);
char motor_back(MOTOR motor);
char motor_right(MOTOR motor);
char motor_left(MOTOR motor);

// Returns current state of motors. 
char motor_state(MOTOR motor);

// Returns FAIL or DISCONNECT on disconnect.
char motor_disconnect(MOTOR *motor);

#endif
