#include <stdio.h>
#include "network.h"
#include "communication.h"



int main (int argc, const char * argv[]) {


    // insert code here...
    printf("Hello, World!\n");
	network_init();

	if(!statemachine_initialize()) {
			printf("Unable to initialize elevator hardware\n");
			return(0);
	}
	while (1) {
		system("clear");
		statemachine_print_state();
		order_print();
		order_add();
		order_handle_button_lamps();
		statemachine(statemachine_get_event());
	}




		//sleep(1);
		//int data [] = {1,2,3};
		//send_msg(OPCODE_NOOP, TOALLIP, data);
/*		struct orders o;
		send_msg(OPCODE_NOOP, TOALLIP, o, 0, 0, data);
*/
/*		struct msg m;
		m.msgtype = OPCODE_IMALIVE;
		m.orderlist.panel_up[0] = 1;

		m.orderlist.panel_up[1] = 1;
		sendtoallpeer(m);*/
		/*struct msg package = {
			.msgtype = OPCODE_NOOP
		};
		sendtoallpeer(package);
		struct peer p = {
				.ip = 0
		};
		sendtopeer(package, p);*/
    return 0;

//
}

