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

static int activeobstr = 0;
int current_pos;


int obstr_on(){
	return activeobstr;
}
int obstr_off(){
	return !activeobstr;
}

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
	// Add to local queue
	n->elevinfo.current_orders[neworder.floor][neworder.paneltype] = 1;

	int gp[] = {floor, value};
	int or[FLOORS][N_PANELS];
	// Broadcast updated new order to all elev on network
	send_msg(OPCODE_NEWORDER, n->elevinfo.ip, or, 0, 0, gp);
	elev_set_button_lamp(value, floor, 1);
}


void operator_callback_sensor(int floor, int value){
	if(value == 1){
		// Entering floor
		elev_set_floor_indicator(floor);
		set_last_floor(floor);
		current_pos = floor;
		set_event(FLOORSENSOR);
		printf("FLOORSENSOR\n");
	}
	else if (value == 0){
		current_pos = BETWEEN_FLOORS;
		set_event(FLOORSENSOR);
		// Leaving floor
	}
	else{
		// Corrupt call

	}
}

int atfloor(){
	return (current_pos!=BETWEEN_FLOORS);
}

int betweenfloors(){
	return (current_pos == BETWEEN_FLOORS);
}
int getcurrentpos(){
	return current_pos;
}
void operator_callback_stop(int floor, int value){
	elev_set_stop_lamp(1);
	set_event(STOP_BUTTON);
	// Push stop event to statemachine
}

void operator_callback_obstr(int floor, int value){
	activeobstr = value;
	set_event(OBSTRUCTION);
}




/* execute orders based on priority
 *
 */
void handle_request(){
	//Check if there is a request at current floor in right direction.
	if(order_check_request_current_floor(ALL)) {
		//do nothing, this is handled in EXECUTE-state.
		;
	}
	//Check if there is a prioritized request above
	else if( order_check_request_above(ALL) && (get_last_dir()==UP || !order_check_request_below(ALL)  )){
		go_up();
	}
	//Check if there is a request in the current direction in current floor
	else if( (get_last_dir()==DOWN && order_check_request_current_floor(CALL_DOWN) ) || (get_last_dir()==UP && order_check_request_current_floor(CALL_UP)) ){
		;
		//do nothing, this is handled in EXECUTE-state.
	}
	//Check if there is an request below
	else if(order_check_request_below(ALL)) {
		go_down();
	}
}

int request_here(){
	//Check if there is a request at current floor in right direction.
	if(order_check_request_current_floor(ALL)) {
		return 1;
	}
	//Check if there is a prioritized request above
	else if( (order_check_request_above(ALL) && get_last_dir()==UP ) || (!order_check_request_below(ALL)) ){
		return 0;
	}
	//Check if there is a request in the current direction in current floor
	else if( (get_last_dir()==DOWN && order_check_request_current_floor(CALL_DOWN)) || (get_last_dir()==UP && order_check_request_current_floor(CALL_UP)) ){
		return 1;
		//do nothing, this is handled in EXECUTE-state.
	}
	//Check if there is an request below
	else if(order_check_request_below(ALL)){
		return 0;
	}
	else{
		return 0;
	}
}





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

void em_stop(){
	// Stop elevator
	elev_set_stop_lamp(1);
	control_slow_down();
	elev_set_speed(0);
}

void em_cancel(){
	elev_set_stop_lamp(0);
}

void em_obstr(){
	elev_set_speed(0);
	if(atfloor()){
		opendoor();
	}
	// Stop elev if moving
	// Open door - keep it open
}
#define DOOROPENTIME 3
struct timeval dooropentime;

void opendoor(){
	control_slow_down();
	elev_set_speed(0);
    gettimeofday(&dooropentime, 0);			//Sets timer.
    order_reset_current_floor();
	elev_set_door_open_lamp(1);
	printf("Door is open \n");
}
void closedoor(){
	elev_set_door_open_lamp(0);
	printf("Door is closed \n");
}

int timeoutdoor(){
	struct timeval currtime;
	gettimeofday(&currtime, 0);
	if( (currtime.tv_sec - dooropentime.tv_sec)>=DOOROPENTIME){
		return 1;
	}
	return 0;
}

