#include <stdio.h>
#include <stdlib.h>
#include "order.h"
#include "control.h"
#include "statemachine.h"

int main()
{
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
	return 0; 
}

