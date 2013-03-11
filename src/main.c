#include <stdio.h>
#include "network.h"




int main (int argc, const char * argv[]) {


	struct msg m;
	m.msgtype = 1;
	m.from 		= 2;
	m.to = 3;
	m.data[0] = 0;
	m.data[1] = 1;
	m.data[2] = 2;
	m.data[3] = 3;

	printf("str: %s\n", struct_to_byte(m));
	m = byte_to_struct(struct_to_byte(m));
	printf("str: %s\n", struct_to_byte(m));


    // insert code here...
    printf("Hello, World!\n");
	network_init();

	while(1){
		sleep(1);
		//	printf("Receivin: %s\n", getbufin());
	}
    return 0;

//
}

