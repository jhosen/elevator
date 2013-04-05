#include <stdio.h>
#include "order.h"
#include "elev.h"

/*****************************          Variables:      **********************************/

static int order_table[N_FLOORS][N_PANELS];     /*Orders are saved in order_table. There are one column for each panel-type(call_down, call_up and 
                                                command). As an example order_table[2][CALL_UP] refers to orders made from call_up-panel in 
                                                3rd floor.*/ 
static enum direction_t last_pass_floor_dir;	// last_pass_floor_dir keeps track of the position of the elevator by remembering the direction the elevator went when passing a floor. 
static int last_floor;							// Used to distinct a new floor from last floor.

/***************************** Get/set functions: **********************************/

void set_last_pass_floor_dir(enum direction_t dir) {
	last_pass_floor_dir = dir;
}

int get_last_floor(){
    return last_floor;
}

void set_last_floor(int floor){
    last_floor = floor;
}

/******************************* Check table functions ******************************************/

int order_check_request_current_floor(enum panel_type panel){
	int current_floor = elev_get_floor_sensor_signal();
	if(current_floor==BETWEEN_FLOORS) 
	    return 0;
	
	/* Wants to check all panels: */
	if(panel==ALL && last_pass_floor_dir==UP){
		if(order_table[current_floor][COMMAND] || order_table[current_floor][CALL_UP]) {
			return 1;
		}
	}
	else if(panel==ALL && last_pass_floor_dir==DOWN) {
		if(order_table[current_floor][COMMAND] || order_table[current_floor][CALL_DOWN]) {
						return 1;
		}
	}
	else 
		return(order_table[current_floor][panel]);
	return 0;
}

int order_check_request_above(enum panel_type panel){
	int current_floor = elev_get_floor_sensor_signal();
	if(current_floor==BETWEEN_FLOORS){			        //This would be of interest if elevator is stopped between two floors due to emergency.
	    if(last_pass_floor_dir==UP){                    //Means elevator is between last_floor and last_floor+1
	        current_floor = last_floor;					//Then the checks are to be done relative to floor recently passed.
	    }
	    else{
	        current_floor = last_floor-1;				//Else check relative to the floor below elevator.
	    }
	} 
	int floor = 0;
	if(panel==ALL){	                                    //If user requests for orders from all panels, then go through the whole table.
		int panel_counter = 0;
		for(panel_counter = CALL_UP; panel_counter<=COMMAND; panel_counter++){
			for(floor = current_floor+1; floor<N_FLOORS; floor ++){
				if(order_table[floor][panel_counter]==1) return 1;
			}
		}	
	}
	else{                                               //Check requests from spesific panel
		for(floor = current_floor+1; floor<N_FLOORS; floor ++){
			if(order_table[floor][panel]==1) return 1;
		}
	}
	return 0;
}

int order_check_request_below(enum panel_type panel){
	int current_floor = elev_get_floor_sensor_signal();
	if(current_floor==BETWEEN_FLOORS){					//This would be of interest if elevator is stopped between two floors due to emergency.
	    if(last_pass_floor_dir==UP){					
	        current_floor = last_floor+1;   			//Then the checks are to be done relative to floor above elevator.
	    }
	    else{                               
	        current_floor = last_floor;     			//Else check requests relative to the last floor passed.
	    }
	}
	int floor = 0;
	if(panel==ALL){			                            //If user requests for orders from all panels, then go-trough the whole table
		int panel_counter = 0;
		for(panel_counter = CALL_UP; panel_counter<=COMMAND; panel_counter++){
			for(floor = current_floor-1; floor>=0; floor--){
				if(order_table[floor][panel_counter]==1) return 1;
			}
		}	
	}
	else{                                               //Check requests from spesific panel
		for(floor = current_floor-1; floor>=0; floor--){
			if(order_table[floor][panel]==1) return 1;
		}
	}
	return 0;
}

/************************* Actions made on order table: **************************************/
void order_add() {
	int floor = 0;
	for(floor=0; floor<N_FLOORS; floor++) {
		if(elev_get_button_signal(BUTTON_COMMAND, floor)) {	
			order_table[floor][COMMAND]=1;
		}
		if(floor!=N_FLOORS-1 && elev_get_button_signal(BUTTON_CALL_UP, floor)) {	//There are no BUTTON_CALL_UP in the highest floor.	
			order_table[floor][CALL_UP] = 1;
		}
		if(floor!=0 && elev_get_button_signal(BUTTON_CALL_DOWN, floor)) {			//There are no BUTTON_CALL_DOWN in the lowest floor.	
				order_table[floor][CALL_DOWN] = 1;
		}		
	}
}


void order_empty(enum panel_type panel){
	int floor = 0;
	if(panel==ALL){
		int panel = 0;
		for(floor=0; floor < N_FLOORS; floor++){
			for(panel = CALL_UP; panel <=COMMAND; panel++){
				order_table[floor][panel]=0;
			}
		}
	}
	else{
		for(floor=0; floor < N_FLOORS; floor++){
			order_table[floor][panel]=0;
		}
	}
}


void order_reset_current_floor(){
	int floor = elev_get_floor_sensor_signal();
	if(floor!=BETWEEN_FLOORS){
		order_table[floor][COMMAND]=0;
	    if(last_pass_floor_dir == UP) {
		    order_table[floor][CALL_UP]=0;
		    if(!(orders_above(floor))) {
		    	order_table[floor][CALL_DOWN]=0;
		    }
	    }
	    else {
	    	order_table[floor][CALL_DOWN] =0;
	    	if(!(orders_below(floor))) {
	    		order_table[floor][CALL_UP]=0;
	    	}
	    }
	}
}

int orders_above(int c_floor) {
	if(c_floor==N_FLOORS) {
		return 0;
	}
	int i = 0;
	int panel = 0;
	for(i=c_floor+1; i<N_FLOORS; i++) {
		for(panel = CALL_UP; panel <=COMMAND; panel++){
			if(order_table[i][panel]) {
			  return 1;
			}
		}
	}
	return 0;
}

int orders_below(int c_floor) {
	if(c_floor==0) {
		return 0;
	}
	int i = 0;
	int panel = 0;
	for(i=c_floor-1; i>=0; i--) {
		for(panel = CALL_UP; panel <=COMMAND; panel++){
			if(order_table[i][panel]) {
			  return 1;
			}
		}
	}
	return 0;
}

/***************************** Other functions *******************************************/

void order_handle_button_lamps(){
    int floor = 0;
    int button = 0;
	for(floor = 0; floor < N_FLOORS; floor++){
	    for(button = BUTTON_CALL_UP; button<=BUTTON_COMMAND; button++){
			if(!((floor==0 && button==BUTTON_CALL_DOWN) || (floor==N_FLOORS-1 && button==BUTTON_CALL_UP))){ 	//Not considering invalid buttons.
			    if(order_table[floor][button]==0)																//Button and panel is represented by corresponing numbers", so we can take button as input for order_table[][]
			       elev_set_button_lamp(button,floor,0);
			    else    
			       elev_set_button_lamp(button, floor, 1);
		    }		
	    }
    }
}

void order_print(void){							
	printf("|	C_UP	|C_DOWN	|COMMAND|\n");	
	int floor = 0;
	int panel = 0;
	for(floor=N_FLOORS-1; floor>=0;floor--){
		printf("Floor %d ", floor+1);		
		for(panel=CALL_UP; panel<=COMMAND; panel++){
			if(!((floor==0 && panel==CALL_DOWN) || (floor==N_FLOORS-1 && panel==CALL_UP)))
			printf("|%d	", order_table[floor][panel]); 
			else 
			printf("|	");
		}
		printf("| \n");
	}
	printf("\n");
}
