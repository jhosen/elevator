/*
 * elevator.h
 *
 *  Created on: Apr 10, 2013
 *      Author: student
 */

#ifndef ELEVATOR_H_
#define ELEVATOR_H_

#include "statemachine.h"


#define N_FLOORS 4
#define N_STATES 5
#define N_EVENTS 4



// {@ States
typedef enum {IDLE, STOP_BUTTON, OBSTRUCTION, FLOORSENSOR} elev_event_t;
//#define IDLE 0
//#define STOP_BUTTON 1
//#define OBSTRUCTION 2
//#define FLOORSENSOR 3
// @}
// {@ Events
typedef enum {EXECUTE=1, EM_STOP, EM_OBSTR, DOOROPEN} elev_state_t;
//#define NOEVENT 0
//#define EXECUTE 1
//#define EM_STOP 2
//#define EM_OBSTR 3
//#define DOOROPEN 4
// @}


/* !\brief Runs elevator into place and starts state machine
 *
 */
void elevator(void);

/* !\brief Used to set elevator event from external function
 *
 */
void set_elev_event(events_t event);

/*	!\brief Runs elevator to floor below
 *
 */
static void init_elev_position(void);

#endif /* ELEVATOR_H_ */
