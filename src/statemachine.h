#ifndef _STATEMACHINE_H
#define _STATEMACHINE_H

#include "pthread.h"


#define NOEVENT 0


typedef int events_t ;
typedef int state_t ;

typedef struct {
	events_t event;
	pthread_mutex_t eventMutex;
} event_t;

typedef struct {
	int nstates;
	int nevents;
} sm_config_t;

struct state_action_pair_t{
	state_t nextState;
	void (*action)(void);
	int (*guard)(void);
};

/* !\brief Init statemachine - set event to NOEVENT and state to IDLE
 *
 * \param eventvar A pointer to the event variable
 * \param statevar A pointer to the state variable
 *
 */
void statemachine_init(event_t * eventvar, state_t *statevar);

/* !\brief Performs an action based on state, event and guard.
 *
 * \param stateTable A pointer to the start of the state machine table
 * \param config holds the number of events and states
 * \param eventvar A pointer to the event variable
 * \param statevar A pointer to the state variable
 *
 */
void statemachine_handleEvent(struct state_action_pair_t * stateTable, sm_config_t config, state_t *statevar, event_t * eventvar);


#endif
