#include <arpa/inet.h>
#include <stdio.h>
#include <signal.h>
#include "drivers/elev.h"
#include "communication.h"
#include "elevator.h"
#include "network.h"

void signal_callback_handler(int signum);


int main () {
	signal(SIGINT, signal_callback_handler);
	signal(SIGTERM, signal_callback_handler);

    elev_init();
    elev_reset_all_lamps();
    elevator_init_pos();
	init_order(inet_addr(getlocalip()));
    network();
    elevator();

    return 0;

}

void signal_callback_handler(int signum){
	printf("\nThank you for elevating with us. We hope to see you again soon.\n");
	elev_set_speed(0);
	int floor, panel;
	for(floor = 0; floor < N_FLOORS; floor ++){
		for(panel = CALL_UP; panel <= COMMAND; panel ++){
			clear_order_all_elev(floor, panel);
		}

	}
	elev_reset_all_lamps();
	exit(signum);
}
