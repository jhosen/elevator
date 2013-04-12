#include <stdio.h>
#include "network.h"
#include "communication.h"
#include <arpa/inet.h>
#include "elevator.h"
#include "elev.h"
#include <signal.h>


void signal_callback_handler(int signum);


int main (int argc, const char * argv[]) {
	signal(SIGINT, signal_callback_handler);
	signal(SIGTERM, signal_callback_handler);

    elev_init();
    elev_reset_all_lamps();
	init_order(inet_addr(getlocalip()));
    network();
    elevator();

    return 0;

}

void signal_callback_handler(int signum){
	printf("\nThank you for elevating with us. We hope to see you again soon.\n");
	elev_set_speed(0);
	elev_reset_all_lamps();
	exit(signum);
}
