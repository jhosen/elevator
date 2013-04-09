#ifndef _CONTROL_H
#define _CONTROL_H
						
/********************************************* DECLARATIONS ****************************************************/
#define ELEV_SPEED 300
#define BETWEEN_FLOORS -1
#define DOOR_OPEN_TIME 3
#define SLOW_DOWN_TIME 8000
/*************************************************FUNCTIONS *****************************************************/

int control_new_floor(void);

/* Ensures a smooth stop in new floors. Slow down time might vary from elevator to elevator. */
void control_slow_down();

int control_time_is_out();

/* Sets timer and opens door. */
void control_open_door();

void control_set_stop_timer();

//void set_dragged_out(flag);

/************************************************ ELEV FUNCTIONS *****************************************************/

/*Functions that are closely connected to the elev functions:*/

void go_up();

void go_down();

void stop_elevator();

void stop_lamp_on();

void stop_lamp_off();

void set_floor_light();

int check_stop();

int get_position();

int check_obstruction();

void close_door();

int init_test(); 

/******************************************** GET/SET FUNCTIONS *******************************************************/

//void set_newly_emergency(int flag);
enum direction_t get_last_dir();
void set_last_dir(enum direction_t dir);












#endif
