/*
 * operator.c
 *
 *  Created on: Apr 7, 2013
 *      Author: student
 */

#include "order.h"
#include "operator.h"
#include "elev.h"
#include "communication.h"
#include "statemachine.h"
#include "control.h"

static int activeobstr = 0;
static int current_pos;

#define DOOROPENTIME 3
struct timeval dooropentime;

void operator_init(){
	elev_register_callback(SIGNAL_TYPE_CALL_UP, 	operator_callback_button);
	elev_register_callback(SIGNAL_TYPE_CALL_DOWN, 	operator_callback_button);
	elev_register_callback(SIGNAL_TYPE_COMMAND, 	operator_callback_button);
	elev_register_callback(SIGNAL_TYPE_SENSOR, 		operator_callback_sensor);
	elev_register_callback(SIGNAL_TYPE_STOP, 		operator_callback_stop);
	elev_register_callback(SIGNAL_TYPE_OBSTR, 		operator_callback_obstr);

	elev_enable_callbacks();
}

//Callback functions

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
	if(value == 1){ // Entering floor
		elev_set_floor_indicator(floor);
		set_last_floor(floor);
		if(floor == FLOORS-1){
			set_last_dir(DOWN);
        }
		else if(floor == 0){
			set_last_dir(UP);
        }
        current_pos = floor;
		set_event(FLOORSENSOR);
        int gpdummy[]={};
        int ordummy[]={};
        send_msg(OPCODE_ELEVSTATE, 0, ordummy, get_last_dir(), floor, gpdummy);

	}
	else if (value == 0){ // Leaving floor
		current_pos = BETWEEN_FLOORS;
		set_event(FLOORSENSOR);
	}
	else{
		// Corrupt call
	}
}
#include "statemachine.h"
void operator_callback_stop(int floor, int value){
	set_event(STOP_BUTTON);
	printf("Stop button was pushed\n");
	// Push stop event to statemachine
}

void operator_callback_obstr(int floor, int value){
	activeobstr = value;
	set_event(OBSTRUCTION);
}

//Get functions
int getcurrentpos(){
	return current_pos;
}

//Functions used as guards by the state machine

int requests(){
	int floor, panel;
	struct node * head = gethead();
	for (panel = CALL_UP; panel<=COMMAND; panel++){
		for(floor = 0; floor< FLOORS; floor ++){
			if(head->elevinfo.current_orders[floor][panel])
				return 1;
		}
	}
	return 0;
}

int should_stop(){
	int current_floor = getcurrentpos();
	if(current_floor==BETWEEN_FLOORS){
		return 0;
	}
	struct node * head = gethead();
	if(get_last_dir()==UP){
		if(head->elevinfo.current_orders[current_floor][COMMAND] || head->elevinfo.current_orders[current_floor][CALL_UP]) {
			return 1;
		}
		else if(!order_check_request_above(ALL)){
			return 1;
		}
	}
	else if(get_last_dir()==DOWN) {
		if(head->elevinfo.current_orders[current_floor][COMMAND] || head->elevinfo.current_orders[current_floor][CALL_DOWN]) {
			return 1;
		}
		else if(!order_check_request_below(ALL)){
			return 1;
		}
	}
	return 0;
}

int timeoutdoor(){
	struct timeval currtime;
	gettimeofday(&currtime, 0);
	if( (currtime.tv_sec - dooropentime.tv_sec)>=DOOROPENTIME){
		return 1;
	}
	return 0;
}

int obstr_on(){
	return activeobstr;
}
int obstr_off(){
	return !activeobstr;
}

int betweenfloors(){
	return (current_pos == BETWEEN_FLOORS);
}

//Functions used as actions by the state machine

void set_direction() {
	int pridir = prioritized_dir();
	if(pridir==1) go_up();
	else if(pridir==0);
	else if(pridir==-1) go_down();
	else;
}

int prioritized_dir(){
	//Check if there is a request at current floor in right direction.
	if(order_check_request_current_floor(ALL)) {
		return 0;
	}
	//Check if there is a request above
	else if(order_check_request_above(ALL) && (get_last_dir()==UP)){
		return 1;
	}
	//Check if there is a request below
	else if(order_check_request_below(ALL) && (get_last_dir()==DOWN)) {
		return -1;
	}
	//Check if there is a request above when the direction is DOWN. This is served if there are no requests below.
	else if(order_check_request_above(ALL) && !(order_check_request_below(ALL))) {
		return 1;
	}
	//Check if there is a request below when the direction is UP. This is served if there are no requests above.
	else if(order_check_request_below(ALL) && !(order_check_request_above(ALL))) {
		return -1;
	}
	else{
		return 0;
	}
}

void opendoor(){
	control_slow_down();
    gettimeofday(&dooropentime, 0);
    order_reset_current_floor();
	elev_set_door_open_lamp(1);
	printf("Door is open \n");
}

void closedoor(){
	elev_set_door_open_lamp(0);
	printf("Door is closed \n");
}

void em_stop(){
	elev_set_stop_lamp(1);
	printf("Stop lamp on\n");
	control_slow_down();
}

void em_cancel(){
	elev_set_stop_lamp(0);
	printf("Stop lamp off\n");

}

void em_obstr(){
	elev_set_speed(0);
	if(getcurrentpos()!=-1){
		opendoor();
	}
	// Stop elev if moving
	// Open door - keep it open
}







