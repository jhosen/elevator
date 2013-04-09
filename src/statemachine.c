#include <stdio.h>
#include "order.h"
#include "control.h"
#include "statemachine.h"
#include "operator.h"

#define N_FLOORS 4
#define N_STATES 5
#define N_EVENTS 4

static enum state_t state = IDLE;
static enum event_t event = NOEVENT;
pthread_mutex_t eventMutex;

void statemachine_init(){
	pthread_mutex_init(&eventMutex, NULL);
	if(elev_get_floor_sensor_signal()==BETWEEN_FLOORS){
		go_down();
		while(elev_get_floor_sensor_signal()==BETWEEN_FLOORS)
			; // wait
	}
	control_slow_down();
	state = IDLE;
}

struct state_action_pair_t{
	enum state_t nextState;
	void (*action)(void);
	int (*guard)(void);
};

/*nextState, action, guard*/
struct state_action_pair_t stateTable[N_STATES][N_EVENTS] = {
/* state|event:	NOEVENT						 			STOP_BUTTON							OBSTRUCTION								FLOORSENSOR*/
/*IDLE		*/ 	{{EXECUTE, set_direction,requests},		{EM_STOP,	em_stop,	NULL},		{EM_OBSTR,	em_stop,	obstr_on},		{IDLE ,	NULL,	NULL}					},
/*EXECUTE	*/	{{DOOROPEN, opendoor,	should_stop},	{EM_STOP,	em_stop,	NULL},		{EM_OBSTR,	em_stop,	obstr_on}, 		{DOOROPEN , opendoor, 	should_stop} 	},
/*EM_STOP	*/	{{EM_STOP, NULL,	NULL},				{IDLE,		em_restart,	obstr_off},	{EM_STOP,	NULL,		NULL},			{EM_STOP, NULL,	NULL} 					},
/*EM_OBSTR	*/	{{EM_OBSTR, NULL,NULL},					{EM_OBSTR,	NULL,		NULL},		{IDLE,		closedoor,	obstr_off}, 	{EM_OBSTR, closedoor, NULL}				},
/*DOOROPEN	*/ 	{{IDLE, closedoor,timeoutdoor},			{EM_STOP, 	em_stop, 	NULL},		{EM_OBSTR, 	obstruction, obstr_on}, 	{IDLE, closedoor, betweenfloors}		},
};


void statemachine_handleEvent(){
	pthread_mutex_lock(&eventMutex);
	struct state_action_pair_t sap = stateTable[state][event];
	event = NOEVENT;
	pthread_mutex_unlock(&eventMutex);
	if(sap.guard == NULL || sap.guard()){
		if(sap.action != NULL){
			sap.action();
		}
		state = sap.nextState;
	}
}

void statemachine(){
	statemachine_init();
	while(1){
		statemachine_handleEvent();

	}

}

void print_state_event(evcpy) {
	printf("State is: ");
	switch(state){
		case IDLE:
			printf("IDLE.\n");
			break;
		case EXECUTE:
			printf("EXECUTE.\n");
			break;
		case DOOROPEN:
			printf("DOOROPEN.\n");
			break;
		case EM_STOP:
			printf("EM_STOP.\n");
			break;
		case EM_OBSTR:
			printf("EM_OBSTR.\n");
			break;
	}
	printf("Event is: ");
	switch(evcpy){
		case NOEVENT:
			printf("NOEVENT.\n");
			break;
		case STOP_BUTTON:
			printf("STOP_BUTTON.\n");
			break;
		case OBSTRUCTION:
			printf("OBSTRUCTION.\n");
			break;
		case FLOORSENSOR:
			printf("FLOORSENSOR.\n");
			break;
	}
}

void set_event(enum event_t evnt){
	pthread_mutex_lock(&eventMutex);
	event = evnt;
	pthread_mutex_unlock(&eventMutex);
}

enum event_t get_event(){
	enum event_t evcopy;
	pthread_mutex_lock(&eventMutex);
	evcopy = event;
	pthread_mutex_unlock(&eventMutex);
	return evcopy;
}
