/*
 * operator.h
 *
 *  Created on: Apr 7, 2013
 *      Author: student
 */

#ifndef OPERATOR_H_
#define OPERATOR_H_

/*
 * operator.c
 *
 *  Created on: Apr 7, 2013
 *      Author: student
 */


void operator_init();


/* !\brief Callback for button hit
 *
 * \param 'floor' is the floor of the button being pushed
 * \param 'value' is the type of button, according to elev_direction_t,
 *  defined in elev.h
 */
void operator_callback_button(int floor, int value);


/* !\brief Callback for sensor
 *
 * \param floor
 * \param value is 1 for entering floor, 0 for leaving
 */
void operator_callback_sensor(int floor, int value);

void operator_callback_stop(int floor, int value);

void operator_callback_obstr(int floor, int value);


int obstr_on();

int obstr_off();

void handle_request();

int request_here();

int requests();
void em_stop();

void em_cancel();

void em_obstr();
void opendoor();
void closedoor();

int timeoutdoor();

int atfloor();
int betweenfloors();
int getcurrentpos();




#endif /* OPERATOR_H_ */
