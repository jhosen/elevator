#include <stdio.h>
#include "network.h"
#include "communication.h"



int main (int argc, const char * argv[]) {


    // insert code here...
    printf("Hello, WorldFOO!\n");
	network_init();


	while(1){
		sleep(1);
		int data [] = {1,2,3};
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
	}
    return 0;

//
}

