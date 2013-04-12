/*
 * elevator.h
 *
 *  Created on: Apr 10, 2013
 *      Author: student
 */

#ifndef ELEVATOR_H_
#define ELEVATOR_H_

#include "utils/statemachine.h"


#define N_FLOORS 4
#define N_PANELS 3
#define N_STATES 5
#define N_EVENTS 4
#define BETWEEN_FLOORS -1
#define MAXWAIT_ORDER_TIME 10


// {@ States
typedef enum {IDLE, STOP_BUTTON, OBSTRUCTION, FLOORSENSOR} elev_event_t;
// @}

// {@ Events
typedef enum {EXECUTE=1, EM_STOP, EM_OBSTR, DOOROPEN} elev_state_t;
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
static void init_elev_position();

#endif /* ELEVATOR_H_ */
