#include <stdio.h>
#include "network.h"

int main (int argc, const char * argv[]) {
    // insert code here...
    printf("Hello, World!\n");
    bufout("Sent from main\0");
	network_init(); 
while(1){
	sleep(1);
	bufout("this is JESPER");
//	printf("Receivin: %s\n", getbufin());
}
	;
    return 0;

//
}

