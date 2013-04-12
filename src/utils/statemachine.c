#include <stdio.h>
#include "statemachine.h"


void statemachine_init(event_t * eventvar, state_t *statevar){
	pthread_mutex_init(&eventvar->eventMutex, NULL);
	eventvar->event = NOEVENT;
	*statevar = 0;
}

void statemachine_handleEvent(struct state_action_pair_t * stateTable, sm_config_t config, state_t *statevar, event_t * eventvar){
	pthread_mutex_lock(&eventvar->eventMutex);
	struct state_action_pair_t sap = stateTable[(*statevar)*config.nevents + eventvar->event];
	eventvar->event = NOEVENT;
	pthread_mutex_unlock(&eventvar->eventMutex);
	if(sap.guard == NULL || sap.guard()){
		if(sap.action != NULL){
			sap.action();
		}
		*statevar = sap.nextState;
	}
}

