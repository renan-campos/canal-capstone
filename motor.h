#ifndef _motor
#define _motor
// Enumerate all message types:
// Sending the handler any of the below states will cause the handler to attempt
// to change the current motor state to the one sent. 
// There are 5 possible motor states:
#define STOPPED   0
#define FORWARD   1
#define BACK      2
#define RIGHT     3
#define LEFT      4

// Sending a STATE will cause the handler to return the current state.
#define STATE     5
// Success is sent by the handler to indicate change in state was made.
#define SUCCESS   6
// Fail is sent by the handler to indicate change in state was not made.
// This can occur when a remote is connected and the auto-pilot tries sending
// a direction.
#define FAIL      7
// This is sent by the remote to let the handler know there is a controller
// other than the auto-pilot
#define CONNECT   8
// This is sent by the remote to let the handler know that the remote is 
// disconnecting.
#define DISCONNECT 9

// A message will have this added to it so that the handler can know who sent it.
#define FROM_REMOTE 10
#define FROM_RCVR   20

#define TO_HNDLR    1
#define TO_SNDR     2

// A message is a simple 1-byte char.
// By keeping the messages so small, byte-ordering is not a problem for network connections.
typedef struct MSG {
	long mtype; // Message type
	char msg;   // Message
} MSG;

// Listens to the message queue and handles messages.
void *msg_handler(void *arg);

// Node Controller port
#define NC_PORT 2018

#endif
