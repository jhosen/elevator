#ifndef _STATEMACHINE_H
#define _STATEMACHINE_H

/********************************************* DECLARATIONS ****************************************************/

enum event_t {NOEVENT, STOP_BUTTON, REQUEST_BELOW, REQUEST_ABOVE, REQUEST_HERE, OBSTRUCTION, NEW_FLOOR};
enum state_t {IDLE, GOING_UP, GOING_DOWN, PASSENGER_ON_OFF,EMERGENCY, SERVICE};

/*************************************************FUNCTIONS *****************************************************/

/* Runs elevator to defined position. No orders are added while initialize is running. 
Initialize returns !=0 if hardware-initializing(elev_init()) went ok. Initialize is used at start-up, and if an obstruction
occurs when elevator is running. */
int statemachine_initialize(void);

/*Returns an event based on inputs from hardware in prioritized order*/
enum event_t statemachine_get_event(void);

/*State machine with event as input*/
void statemachine(enum event_t event);

/*Prints current state for supervising reasons*/
void statemachine_print_state(void);

/*Get/set functions*/
enum state_t get_state();
void set_state(enum state_t new_state);

#endif
