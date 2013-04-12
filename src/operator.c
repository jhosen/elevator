/*
 * operator.c
 *
 *  Created on: Apr 7, 2013
 *      Author: student
 */

#include "communication.h"
#include "control.h"
#include "elevator.h"
#include "elev.h"
#include "order.h"
#include "operator.h"


void operator_init(){
	elev_register_callback(SIGNAL_TYPE_CALL_UP, 	operator_callback_button);
	elev_register_callback(SIGNAL_TYPE_CALL_DOWN, 	operator_callback_button);
	elev_register_callback(SIGNAL_TYPE_COMMAND, 	operator_callback_button);
	elev_register_callback(SIGNAL_TYPE_SENSOR, 		operator_callback_sensor);
	elev_register_callback(SIGNAL_TYPE_STOP, 		operator_callback_stop);
	elev_register_callback(SIGNAL_TYPE_OBSTR, 		operator_callback_obstr);
	elev_enable_callbacks();
}

void operator_callback_button(int floor, int value){

	struct order neworder = {.floor = floor,.paneltype = value, };

	struct node * n;
	if(value == ELEV_DIR_COMMAND){
		n = gethead();
	}
	else{
		n = weightfunction(gethead(), neworder);
	}
    order_register_new_order(n, neworder.floor, neworder.paneltype);
}

void operator_callback_sensor(int floor, int value){
	if((floor == 0 || floor == N_FLOORS-1) && value == 1)
		elev_set_speed(0);
	if(value == 1){ // Entering floor
		elev_set_floor_indicator(floor);
		set_last_floor(floor);
		if(floor == N_FLOORS-1){
			set_last_dir(DOWN);
        }
		else if(floor == 0){
			set_last_dir(UP);
        }
		control_setcurpos(floor);
        set_elev_event(FLOORSENSOR);
        int gpdummy[]={};
        send_msg(OPCODE_ELEVSTATE, 0, get_last_dir(), floor, gpdummy);

	}
	else if (value == 0){ // Leaving floor
		control_setcurpos(BETWEEN_FLOORS);
		set_elev_event(FLOORSENSOR);
	}
	else{
		// Corrupt call
	}
}

void operator_callback_stop(int floor, int value){
	set_elev_event(STOP_BUTTON);
}

void operator_callback_obstr(int floor, int value){
	setactiveobstr(value);
	set_elev_event(OBSTRUCTION);
}
