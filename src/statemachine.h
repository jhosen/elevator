#ifndef _STATEMACHINE_H
#define _STATEMACHINE_H

/********************************************* DECLARATIONS ****************************************************/

typedef enum event_t {NOEVENT, STOP_BUTTON, OBSTRUCTION, FLOORSENSOR};
typedef enum state_t {IDLE, EXECUTE, EM_STOP, EM_OBSTR, DOOROPEN};





void statemachine_init();

void statemachine_handleEvent(enum event_t event);

void statemachine();

void set_event(enum event_t evnt);

enum event_t get_event();

///*************************************************FUNCTIONS *****************************************************/
//
///* Runs elevator to defined position. No orders are added while initialize is running.
//Initialize returns !=0 if hardware-initializing(elev_init()) went ok. Initialize is used at start-up, and if an obstruction
//occurs when elevator is running. */
//int statemachine_initialize(void);
//
///*Returns an event based on inputs from hardware in prioritized order*/
//enum event_t statemachine_get_event(void);
//
///*State machine with event as input*/
//void statemachine(enum event_t event);
//
///*Prints current state for supervising reasons*/
//void statemachine_print_state(void);
//
///*Get/set functions*/
//enum state_t get_state();
//void set_state(enum state_t new_state);

#endif
