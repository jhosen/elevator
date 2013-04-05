#include <stdio.h>
#include "order.h"
#include "control.h"
#include "statemachine.h"

#define N_FLOORS 4


/************************************ Variables **************************************/
static enum state_t state = IDLE;

/************************************ Get-event **************************************/

enum event_t statemachine_get_event(void){ 	//Returns events in prioritized order
	printf("Event: ");             
	
	if(state == EMERGENCY && control_new_floor() ){ // Highest priority if state is EMERGENCY
		printf("NEW FLOOR\n");
		return NEW_FLOOR;     		
	}
	else if(check_stop()){  	
	    printf("STOP\n");
		return STOP_BUTTON;
	}
	else if(check_obstruction()){
	    printf("OBSTRUCTION\n");
		return OBSTRUCTION;
    }
	else if(control_new_floor()){
		printf("NEW FLOOR\n");
		return NEW_FLOOR;     		
	}
	else if(order_check_request_current_floor(ALL)){
		printf("REQUEST HERE\n");
		return REQUEST_HERE;
	}
	else if(order_check_request_above(ALL)){
	    printf("REQUEST ABOVE\n");
		return REQUEST_ABOVE;
	}
	else if(order_check_request_below(ALL)){
		printf("REQUEST BELOW\n"); 				 
		return REQUEST_BELOW;  
	}
	else {
	    printf("NOEVENT\n");
	    return NOEVENT;
	    }
}


/****************************** Statemachine *********************************/

void statemachine(enum event_t event){
	switch (state) {
		case IDLE:
			switch (event) {
				case STOP_BUTTON:
					stop_lamp_on();
					control_open_door();
					order_empty(ALL); 					
					set_newly_emergency(1);	//Setting newly_emergency flag to true, so that control_new_floor will recognize all floors as a "new floor". 
					state = EMERGENCY;
					break;
				case REQUEST_ABOVE:
					go_up();				
					state = GOING_UP;
					break;
				case REQUEST_BELOW:
					go_down();
					state = GOING_DOWN;
					break;
				case REQUEST_HERE:
					control_open_door();
					order_reset_current_floor(); 
					state = PASSENGER_ON_OFF;				        
					break;
				case OBSTRUCTION:
					state = SERVICE; 		//If obstruction occure when door is closed, something is terribly wrong and elevator must be serviced. 
					break;
				case NOEVENT:
					if(get_position()==-1) {
						set_dragged_out(1);
					}
				case NEW_FLOOR:
				default:
					break;
			}
			break;
		case PASSENGER_ON_OFF:
			if(get_position()==-1) {
				close_door();
				state = IDLE;
				break;
			}
			switch (event) {
				case STOP_BUTTON:
					stop_lamp_on();
					control_open_door();
					order_empty(ALL); 
					set_newly_emergency(1);	//Setting newly_emergency flag to true, so that control_new_floor will recognize all floors as a "new floor". 
					state = EMERGENCY;
				case REQUEST_ABOVE:
					order_reset_current_floor(); 					
					/* PRIORITY_TAG: This if-structure is added because of priority from statemachine_get_event:*/
					if(control_time_is_out() && (get_last_dir()==UP || !order_check_request_below(ALL))){       	//If elevator is going up or there are no requests below, requests above got 1. priority.
						close_door();        
       					printf("Door is closed \n"); 						
						go_up();                             														
						state = GOING_UP;
						break;
					}					//Otherwise elevator is going down and there are requests below, requests below got 1. priority.
				case REQUEST_BELOW:
					order_reset_current_floor(); 					
					if(control_time_is_out()) {
						close_door();        
       					printf("Door is closed \n"); 					    
						go_down();
					    state = GOING_DOWN;
					}
					break;
				case REQUEST_HERE:
					control_open_door();
					order_reset_current_floor(); 
					break;
				case OBSTRUCTION:
					control_open_door();
					order_reset_current_floor(); 				
					break;
				case NOEVENT:
					order_reset_current_floor(); 
				    if(control_time_is_out() || get_position()==BETWEEN_FLOORS) {
						close_door();        
       					printf("Door is closed \n"); 
					    state = IDLE;
					}
					break;
				case NEW_FLOOR:
					set_last_floor(get_position());
					break;
				default:
					break;
			}
			break;
		case GOING_DOWN:
			switch (event) {
				case STOP_BUTTON:
					control_slow_down(-ELEV_SPEED);
					stop_lamp_on();
					order_empty(ALL);
					set_newly_emergency(1);	//Setting newly_emergency flag to true, so that control_new_floor will recognize all floors as a "new floor". 
					state = EMERGENCY;
					break;
				case NEW_FLOOR:
					set_floor_light();	
					if(order_check_request_current_floor(COMMAND)	||			// Guard - Stops if user inside elevator has requested spesific floor,  
					   order_check_request_current_floor(CALL_DOWN)	||			// or if someone wants to go down, 
					   (order_check_request_current_floor(CALL_UP)				// or if someone wants to go up and 
						&& !order_check_request_below(ALL)))		{			// current floor holds the lowest request in the system.

						control_slow_down(-ELEV_SPEED);
						control_open_door();
						order_reset_current_floor(); 
						state = PASSENGER_ON_OFF;
					}
					else if((order_check_request_above(ALL) && !order_check_request_below(ALL)) || get_position()==0){  //If elevator has been pulled out of defined floor it cannot know if it is above or below last floor.
						                                                                                                //Then it checks at next floor and at lowest floor if it should turn around.
						control_slow_down(-ELEV_SPEED);                                                                 
						go_up();
						state = GOING_UP;
					}
					break;					
				case OBSTRUCTION:
					control_slow_down(-ELEV_SPEED);
					state = SERVICE;	//if an obstruction occurs while elevator is moving - something is wrong - and service is needed!
					break;
				case NOEVENT:
				case REQUEST_HERE:
				case REQUEST_ABOVE:
				case REQUEST_BELOW:
				default:
					break;		
			}
			break;

		case GOING_UP:
			switch (event){	
				case STOP_BUTTON:
					control_slow_down(ELEV_SPEED);
					stop_lamp_on();
					order_empty(ALL);
					set_newly_emergency(1);	//Setting newly_emergency flag to true, so that control_new_floor will recognize all floors as a "new floor". 
					state = EMERGENCY;
					break;
				case NEW_FLOOR:
					set_floor_light();			
					if (order_check_request_current_floor(COMMAND)||			// Guard - Stops if user inside elevator has requested spesific floor, 
					   order_check_request_current_floor(CALL_UP) ||			// or if someone wants to go up, 
					   (order_check_request_current_floor(CALL_DOWN)			// or if someone wants to go down and  
						&& !order_check_request_above(ALL)))		{			// current floor holds highest request in system.
						   
						control_slow_down(ELEV_SPEED);
						control_open_door();
						order_reset_current_floor(); 
						state = PASSENGER_ON_OFF;
					}
					else if((order_check_request_below(ALL) && !order_check_request_above(ALL)) || get_position()==N_FLOORS-1){  //If elevator has been pulled out of defined floor it cannot know if it is above or below last floor.
						                                                                                                         //Then it checks at next floor and at highest floor if it should turn around.
						control_slow_down(ELEV_SPEED);
						go_down();
						state = GOING_DOWN;
					}
					break;				
				case OBSTRUCTION:
					control_slow_down(ELEV_SPEED);
					state = SERVICE;	//if an obstruction occurs while elevator is moving - something is wrong - and service is needed!
					break;
				case NOEVENT:
				case REQUEST_HERE:
				case REQUEST_ABOVE:
				case REQUEST_BELOW:
				default:
					break;
			}
			break;
		case EMERGENCY:	
		    switch(event){
		        case REQUEST_HERE:
		            if(order_check_request_current_floor(COMMAND)){ 
						order_empty(CALL_UP);
						order_empty(CALL_DOWN);		                
						stop_lamp_off();
		                set_newly_emergency(0);
		               	control_open_door();
						order_reset_current_floor(); 
		                state = PASSENGER_ON_OFF;
		               	break;
		            }
				case REQUEST_ABOVE:
                    if(order_check_request_above(COMMAND)){			
						order_empty(CALL_UP);
						order_empty(CALL_DOWN);    		            
						stop_lamp_off();
    		            close_door();								
                        go_up();
                        state = GOING_UP;
                        break;
                    }
                case REQUEST_BELOW:
					order_empty(CALL_UP);
					order_empty(CALL_DOWN);                     
					if(order_check_request_below(COMMAND)){			
    		            stop_lamp_off(); 
    		            close_door();							
                        go_down();            
                        state = GOING_DOWN;
                    }
                    break;
                case NEW_FLOOR:								
					order_empty(CALL_UP);
					order_empty(CALL_DOWN);                     
					break;
				case OBSTRUCTION:                                 
                case NOEVENT:        
                case STOP_BUTTON:	
                default: 
                	if(get_position()==BETWEEN_FLOORS)			// If elevator passes one floor while stopping, newly_emergency flag must be set to true. 
    					set_newly_emergency(1);					// newly_emergency might already have been set to false when passing the floor.
                												// In that way all floors(even last_floor) will be recognized as a new floor.
					order_empty(CALL_UP);
					order_empty(CALL_DOWN); 
					break;                                                          
		    }									            
		    break;

		case SERVICE:
			statemachine_initialize();						// Reinitializing elevator.			
			state = PASSENGER_ON_OFF;			
			break;
	}	
}
/*************************** Initialize function ******************************/

int statemachine_initialize(void){
	int init_ok = init_test();							
	order_empty(ALL);											
	if(get_position()==BETWEEN_FLOORS){	
		go_down();													
		while(get_position()==BETWEEN_FLOORS){
		/*Do nothing but wait for reaching floor*/
		}
		control_slow_down(-ELEV_SPEED);
	}
	set_floor_light(get_position());											
	set_last_floor(get_position());							
	return init_ok;											
}
/******************************************************************************/

void statemachine_print_state(void){
	printf("State is ");
	switch(get_state()){
		case IDLE:
			printf("IDLE\n");
			break;
		case PASSENGER_ON_OFF:
			printf("PASSENGER_ON_OFF\n");
			break;
		case GOING_UP:
			printf("GOING_UP\n");
			break;
		case GOING_DOWN:
			printf("GOING_DOWN\n");
			break;
		case EMERGENCY:
			printf("EMERGENCY\n");
			break;
		case SERVICE:
			printf("SERVICE\n");
			break;
	}
}


/*******************Set/get-functions******************/
enum state_t get_state(){
    return state;
}
void set_state(enum state_t new_state){
    state = new_state;
}
