/*
 * elevator.c
 *
 *  Created on: Apr 10, 2013
 *      Author: student
 */

#include "statemachine.h"
#include "control.h"
#include "operator.h"
#include "elevator.h"


static state_t state;
static event_t ev;
static sm_config_t elevconf = {.nstates = N_STATES, .nevents=N_EVENTS};


struct state_action_pair_t stateTable[N_STATES][N_EVENTS] = {
/* state|event:	NOEVENT						 			STOP_BUTTON							OBSTRUCTION								FLOORSENSOR*/
/*IDLE		*/ 	{{EXECUTE, set_direction,requests},		{EM_STOP,	em_stop,	NULL},		{EM_OBSTR,	em_stop,	obstr_on},		{IDLE ,	NULL,	NULL}					},
/*EXECUTE	*/	{{DOOROPEN, opendoor,	should_stay},	{EM_STOP,	em_stop,	NULL},		{EM_OBSTR,	em_stop,	obstr_on}, 		{DOOROPEN , opendoor, 	should_stop} 	},
/*EM_STOP	*/	{{EM_STOP, NULL,	NULL},				{IDLE,		em_restart,	obstr_off},	{EM_STOP,	NULL,		NULL},			{EM_STOP, NULL,	NULL} 					},
/*EM_OBSTR	*/	{{EM_OBSTR, NULL,NULL},					{EM_OBSTR,	NULL,		NULL},		{IDLE,		closedoor,	obstr_off}, 	{EM_OBSTR, closedoor, NULL}				},
/*DOOROPEN	*/ 	{{IDLE, closedoor,timeoutdoor},			{EM_STOP, 	em_stop, 	NULL},		{EM_OBSTR, 	obstruction, obstr_on}, 	{IDLE, closedoor, betweenfloors}		},
};

void elevator(){
	operator_init();
	init_elev_position();
	statemachine_init(&state, &ev);
	while(1){
		statemachine_handleEvent(&stateTable, elevconf, &state, &ev);
	}
}


static void init_elev_position(){
	if(elev_get_floor_sensor_signal()==BETWEEN_FLOORS){
		go_down();
		while(elev_get_floor_sensor_signal()==BETWEEN_FLOORS)
			; // wait
	}
	control_slow_down();
}


void set_elev_event(events_t event){
	pthread_mutex_lock(&ev.eventMutex);
	ev.event = event;
	pthread_mutex_unlock(&ev.eventMutex);
}


