/*
 * elevator.c
 *
 *  Created on: Apr 10, 2013
 *      Author: student
 */
#include "elevator.h"
#include "drivers/elev.h"
#include "utils/statemachine.h"
#include "control.h"
#include "operator.h"
#include "order.h"

static state_t state;
static event_t ev;
static sm_config_t elevconf = {.nstates = N_STATES, .nevents=N_EVENTS};

struct state_action_pair_t stateTable[N_STATES][N_EVENTS] = {
/* state|event:	NOEVENT						 			STOP_BUTTON							OBSTRUCTION								FLOORSENSOR*/
/*IDLE		*/ 	{{EXECUTE, 	control_setdir,		order_requests},	{EM_STOP,	control_emergency,	NULL},		{EM_OBSTR,	control_emergency,	obstr_on},		{IDLE ,		NULL,				NULL}				},
/*EXECUTE	*/	{{DOOROPEN, control_executeorder,order_should_stay},{EM_STOP,	control_emergency,	NULL},		{EM_OBSTR,	control_emergency,	obstr_on}, 		{DOOROPEN , control_executeorder, order_should_stop} },
/*EM_STOP	*/	{{EM_STOP, 	NULL,				NULL},				{IDLE,		control_emrestart,	obstr_off},	{EM_STOP,	NULL,				NULL},			{EM_STOP, 	NULL,				NULL} 				},
/*EM_OBSTR	*/	{{EM_OBSTR, NULL,				NULL},				{EM_OBSTR,	NULL,				NULL},		{IDLE,		control_emrestart,	obstr_off}, 	{EM_OBSTR, 	control_closedoor, 	NULL}				},
/*DOOROPEN	*/ 	{{IDLE, 	control_closedoor,control_timeoutdoor}, {EM_STOP, control_emergency, 	NULL},		{EM_OBSTR, 	control_obstr, 		obstr_on}, 		{IDLE, 		control_closedoor, 	control_betweenfloors}},
};

void elevator(){
	operator_init();
	statemachine_init(&state, &ev);
	while(1){
		statemachine_handleEvent(&stateTable, elevconf, &state, &ev);
	}
}

void elevator_init_pos(){
	if(elev_get_floor_sensor_signal()==BETWEEN_FLOORS){
		control_down();
		while(elev_get_floor_sensor_signal()==BETWEEN_FLOORS)
			; // wait
	}
	int floor = elev_get_floor_sensor_signal();
	set_last_floor(floor);
	control_setcurpos(floor);
	control_stop();
}

void set_elev_event(events_t event){
	pthread_mutex_lock(&ev.eventMutex);
	ev.event = event;
	pthread_mutex_unlock(&ev.eventMutex);
}
