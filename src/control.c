#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include "drivers/elev.h"
#include "control.h"
#include "order.h"
#include "communication.h"


static enum direction_t last_dir;
struct timeval dooropentime;
static int current_pos;
static int activeobstr = 0;


void control_setcurpos(int position){
	current_pos = position;
}
int control_getcurpos(){
	return current_pos;
}

void control_up(){
	last_dir=UP;
	elev_set_speed(ELEV_SPEED);
}

void control_down(){
	last_dir=DOWN;
	elev_set_speed(-ELEV_SPEED);
}

void control_stop() {
	int current_speed = io_read_analog(0);
	elev_set_speed(2048-current_speed);
	usleep(SLOW_DOWN_TIME);
	elev_set_speed(0);

}

void control_executeorder(){
	control_stop();
    gettimeofday(&dooropentime, 0);
    order_reset_current_floor();
	elev_set_door_open_lamp(1);
}

void control_closedoor(){
	elev_set_door_open_lamp(0);
}


void control_setdir(){
	int pridir = order_pridir();
	if(pridir==1) control_up();
	else if(pridir==-1) control_down();
	else;
}

int control_betweenfloors(){
	return (control_getcurpos() == BETWEEN_FLOORS);
}

int control_timeoutdoor(){
	struct timeval currtime;
	gettimeofday(&currtime, 0);
	if( (currtime.tv_sec - dooropentime.tv_sec)>=DOOROPENTIME){
		return 1;
	}
	return 0;
}

void control_emergency(){
	elev_set_stop_lamp(1);
	control_stop();
	struct node * elevlistroot = gethead();
	if(count(elevlistroot)>1){
		int gpdummy[]={0};
		send_msg(OPCODE_ELEVINEMERGENCY, elevlistroot->next->elevinfo.ip, 0, 0, gpdummy);
		ordertablemerge(elevlistroot->next, elevlistroot, CALL_UP);
		ordertablemerge(elevlistroot->next, elevlistroot, CALL_DOWN);
		order_flush_panel(elevlistroot, CALL_UP);
		order_flush_panel(elevlistroot, CALL_DOWN);
		deactivate(elevlistroot, *elevlistroot);
	}
}

void control_emrestart(){
	elev_set_door_open_lamp(0);
	struct node * elevlistroot = gethead();
	if(count(elevlistroot)>1){
		int gpdummy[]={0};
		send_msg(OPCODE_ELEV_NOT_EMERGENCY, elevlistroot->next->elevinfo.ip, 0, 0, gpdummy);
	}
	activate(elevlistroot, *elevlistroot);
	elev_set_stop_lamp(0);

}

void control_obstr() {
	control_stop();
	struct node * elevlistroot = gethead();
	if(count(elevlistroot)>1){
		int gpdummy[]={0};
		send_msg(OPCODE_ELEVINEMERGENCY, elevlistroot->next->elevinfo.ip, 0, 0, gpdummy);
		ordertablemerge(elevlistroot->next, elevlistroot, CALL_UP);
		ordertablemerge(elevlistroot->next, elevlistroot, CALL_DOWN);
		order_flush_panel(elevlistroot, CALL_UP);
		order_flush_panel(elevlistroot, CALL_DOWN);
		deactivate(elevlistroot, *elevlistroot);
	}
	if(control_getcurpos()!=-1){
		control_executeorder();
	}
}

enum direction_t get_last_dir(){
	return last_dir;
}

void set_last_dir(enum direction_t dir){
	last_dir = dir;
}

void setactiveobstr(int value){
	activeobstr = value;
}

int obstr_on(){
	return activeobstr;
}
int obstr_off(){
	return !activeobstr;
}
