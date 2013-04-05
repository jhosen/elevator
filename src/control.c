#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include "control.h"
#include "elev.h"
#include "order.h"
/************************************ Variables **************************************/

static time_t timer, t_now, timer_stop;
static enum direction_t last_dir;                       //Keeps information about last direction.
static int newly_emergency=0;                           /*Flag that keeps information about if elevator just came from emergency state.
									                    Flag is used in control_new_floor when coming from emergency state between two floors to detect a new
									                    floor even though it might be the same as the last one passed.*/
static int dragged_out =0;

/************************************ ELEV FUNCTIONS *****************************************************/
/*Functions that are closely connected to the functions from elev.c:*/

void go_up(){
	last_dir=UP;
	elev_set_speed(ELEV_SPEED);
}

void go_down(){
	last_dir=DOWN;
	elev_set_speed(-ELEV_SPEED);
}

void stop_elevator(){
	elev_set_speed(0);
}

void stop_lamp_on(){
	elev_set_stop_lamp(1);
}

void stop_lamp_off(){
	elev_set_stop_lamp(0);
}

void set_floor_light(){
	elev_set_floor_indicator(elev_get_floor_sensor_signal());
}

int check_stop(){
	return elev_get_stop_signal();
}

int check_obstruction(){
	return elev_get_obstruction_signal();
}

int get_position(){
	return elev_get_floor_sensor_signal();
}

void close_door(){
	elev_set_door_open_lamp(0);     
}

int init_test() {
	return elev_init();
}

/*********************************** Functions ***********************************************/

int control_new_floor(){
	int new_floor=get_position();
	if(new_floor!=BETWEEN_FLOORS){      						//If being at a defined floor
	    set_last_pass_floor_dir(last_dir);						//last_pass_floor_dir is updated.
	    if(new_floor!=get_last_floor() || newly_emergency ||dragged_out){        //If the elevator has reached new floor or someone has recently pushed the STOP_BUTTON,
			set_last_floor(new_floor);							//and last_floor is updated.
            set_newly_emergency(0);				                //If elevator is stopped at floor, there is no need for newly_emergency flag for detecting new floors, because floor is defined.
			set_dragged_out(0);
            return 1;
	    }
	}

	return 0;
}

void control_slow_down(int current_speed) {
	elev_set_speed(-current_speed);	
	usleep(SLOW_DOWN_TIME);
	stop_elevator();
}

void control_open_door(){
    (void) time(&timer);			//Sets timer.
	elev_set_door_open_lamp(1);
	printf("Door is open \n"); 
}

void control_set_stop_timer() {
	(void) time(&timer_stop);
}

int control_time_is_out(){
    (void) time(&t_now);
    return(t_now-timer>DOOR_OPEN_TIME);		//Checks if timer has run out.
}

/************************************* Get/set functions *********************************************/

void set_newly_emergency(int flag){
	newly_emergency = flag;
}

void set_dragged_out(int flag) {
	dragged_out = flag;
}

enum direction_t get_last_dir(){
	return last_dir;
}

void set_last_dir(enum direction_t dir){
	last_dir = dir;
}

