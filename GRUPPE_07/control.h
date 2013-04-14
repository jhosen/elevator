#ifndef _CONTROL_H
#define _CONTROL_H
						
#define ELEV_SPEED 		300
#define BETWEEN_FLOORS	-1
#define DOOROPENTIME 3
#define SLOW_DOWN_TIME 	8000


int control_getcurpos();

void control_up();

void control_down();

void control_stop();

void control_setcurpos(int position);

void control_executeorder();

void control_closedoor();

void control_emergency();

void control_emrestart();

void control_obstr();

void control_setdir();

int control_betweenfloors();

int control_timeoutdoor();

int obstr_on();

int obstr_off();

void setactiveobstr(int value);

enum direction_t get_last_dir();

void set_last_dir(enum direction_t dir);












#endif
