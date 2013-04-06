#include <stdio.h>
#include "order.h"
#include "elev.h"
#include "communication.h"

static struct node head;


void init_order(){
	initlist(&head);
}

/** Linked list for keeping track of connected elevators **/


void initlist(struct node * root){
	root = malloc( sizeof(struct node) );
    root->next 		= 0;
    root->prev		= 0;
}
int printlist(struct node * root){
	struct node * iter;
	iter = root;
	int i = 0;
	if(iter!=0){
		while(iter!=0){
			printf("Node %i, id = %i\n", i, iter->elevinfo.ip);
			iter = iter->next;
			i++;
		}
	}
	return 1;
}

int count(struct node * root){
	struct node * iter;
	iter = root;
	int i = 0;
	if(iter!=0){
		while(iter!=0){
			iter = iter->next;
			i++;
		}
	}
	return i; // Does also count the root
}

int add(struct node * root, struct node * new){
	struct node * iter, *prev;
	iter = root;
	if(iter!=0){
		while(iter->next!=0){
			iter = iter->next;
		}
	}
	iter->next = new;
	prev = iter;
	iter = iter->next;
	if(iter==0){
		return 0; //out of memory
	}
	iter->next  	= 0;
	iter->prev		= prev;
	return 1; // success
    
}

int rm(struct node* root, struct node n){
	struct node * iter, *prev, *tmp;
	iter = root;
	while(iter!=0){
		if((iter->elevinfo.ip) == n.elevinfo.ip){
			tmp = iter;
			iter->prev->next = iter->next;
			if(iter->next!=0){
				iter->next->prev = iter->prev;
			}
			free(tmp);
			return 1;
		}
		prev = iter;
		iter = iter->next;
	}
	return 0;
}


int find(struct node * root, struct node n){
	struct node * iter;
	iter = root;
	while(iter!=0){
		if((iter->elevinfo.ip) == n.elevinfo.ip){
			return 1; // found it
		}
		iter = iter->next;
	}
	return 0;
}

struct node * get(struct node * root, struct node n){
	struct node * iter;
	iter = root;
	while(iter!=0){
		if((iter->elevinfo.ip) == n.elevinfo.ip){
			return iter;
		}
		iter = iter->next;
	}
	return 0;
}


/** Elevator list     **/

struct node * getelevnode(struct node n){
	return get(&head, n);
}
void addelev(struct elevator elev){
	struct node * n = malloc(sizeof(struct node));
	n->elevinfo = elev;
	add(&head, n);

}


void addorder(struct node * elevnode, struct order ordr){
	switch (ordr.paneltype){
	case PANEL_CMD:
		elevnode->elevinfo.current_orders.panel_cmd[ordr.floor] = 1;
		break;
	case PANEL_UP:
		elevnode->elevinfo.current_orders.panel_up[ordr.floor] = 1;
		break;
	case PANEL_DOWN:
		elevnode->elevinfo.current_orders.panel_down[ordr.floor] = 1;
		break;
	}
}


/*****************************          Variables:      **********************************/




static int order_table[N_FLOORS][N_PANELS];     /*Orders are saved in order_table. There are one column for each panel-type(call_down, call_up and 
                                                command). As an example order_table[2][CALL_UP] refers to orders made from call_up-panel in 
                                                3rd floor.*/ 
static enum direction_t last_pass_floor_dir;	// last_pass_floor_dir keeps track of the position of the elevator by remembering the direction the elevator went when passing a floor. 
static int last_floor;							// Used to distinct a new floor from last floor.

/***************************** Get/set functions: **********************************/

void set_last_pass_floor_dir(enum direction_t dir) {
	last_pass_floor_dir = dir;
}

int get_last_floor(){
    return last_floor;
}

void set_last_floor(int floor){
    last_floor = floor;
}

/******************************* Check table functions ******************************************/

int order_check_request_current_floor(enum panel_type panel){
	int current_floor = elev_get_floor_sensor_signal();
	if(current_floor==BETWEEN_FLOORS) 
	    return 0;
	
	/* Wants to check all panels: */
	if(panel==ALL && last_pass_floor_dir==UP){
		if(order_table[current_floor][COMMAND] || order_table[current_floor][CALL_UP]) {
			return 1;
		}
	}
	else if(panel==ALL && last_pass_floor_dir==DOWN) {
		if(order_table[current_floor][COMMAND] || order_table[current_floor][CALL_DOWN]) {
						return 1;
		}
	}
	else 
		return(order_table[current_floor][panel]);
	return 0;
}

int order_check_request_above(enum panel_type panel){
	int current_floor = elev_get_floor_sensor_signal();
	if(current_floor==BETWEEN_FLOORS){			        //This would be of interest if elevator is stopped between two floors due to emergency.
	    if(last_pass_floor_dir==UP){                    //Means elevator is between last_floor and last_floor+1
	        current_floor = last_floor;					//Then the checks are to be done relative to floor recently passed.
	    }
	    else{
	        current_floor = last_floor-1;				//Else check relative to the floor below elevator.
	    }
	} 
	int floor = 0;
	if(panel==ALL){	                                    //If user requests for orders from all panels, then go through the whole table.
		int panel_counter = 0;
		for(panel_counter = CALL_UP; panel_counter<=COMMAND; panel_counter++){
			for(floor = current_floor+1; floor<N_FLOORS; floor ++){
				if(order_table[floor][panel_counter]==1) return 1;
			}
		}	
	}
	else{                                               //Check requests from spesific panel
		for(floor = current_floor+1; floor<N_FLOORS; floor ++){
			if(order_table[floor][panel]==1) return 1;
		}
	}
	return 0;
}

int order_check_request_below(enum panel_type panel){
	int current_floor = elev_get_floor_sensor_signal();
	if(current_floor==BETWEEN_FLOORS){					//This would be of interest if elevator is stopped between two floors due to emergency.
	    if(last_pass_floor_dir==UP){					
	        current_floor = last_floor+1;   			//Then the checks are to be done relative to floor above elevator.
	    }
	    else{                               
	        current_floor = last_floor;     			//Else check requests relative to the last floor passed.
	    }
	}
	int floor = 0;
	if(panel==ALL){			                            //If user requests for orders from all panels, then go-trough the whole table
		int panel_counter = 0;
		for(panel_counter = CALL_UP; panel_counter<=COMMAND; panel_counter++){
			for(floor = current_floor-1; floor>=0; floor--){
				if(order_table[floor][panel_counter]==1) return 1;
			}
		}	
	}
	else{                                               //Check requests from spesific panel
		for(floor = current_floor-1; floor>=0; floor--){
			if(order_table[floor][panel]==1) return 1;
		}
	}
	return 0;
}

/************************* Actions made on order table: **************************************/
void order_add_order(struct order ord){
	order_table[ord.floor][ord.paneltype] = 1;
}
void order_add() {
	int floor = 0;
	for(floor=0; floor<N_FLOORS; floor++) {
		if(elev_get_button_signal(BUTTON_COMMAND, floor)) {	
			order_table[floor][COMMAND]=1;
		}
		if(floor!=N_FLOORS-1 && elev_get_button_signal(BUTTON_CALL_UP, floor)) {	//There are no BUTTON_CALL_UP in the highest floor.	
			order_table[floor][CALL_UP] = 1;
			struct orderlist or;
			int gp[] = {floor, PANEL_UP};
			send_msg(OPCODE_NEWORDER, TOALLIP, or, 0, 0, gp);
		}
		if(floor!=0 && elev_get_button_signal(BUTTON_CALL_DOWN, floor)) {			//There are no BUTTON_CALL_DOWN in the lowest floor.	
			order_table[floor][CALL_DOWN] = 1;
			struct orderlist or;
			int gp[] = {floor, PANEL_DOWN};
			send_msg(OPCODE_NEWORDER, TOALLIP, or, 0, 0, gp);
		}		
	}
}


void order_empty(enum panel_type panel){
	int floor = 0;
	if(panel==ALL){
		int panel = 0;
		for(floor=0; floor < N_FLOORS; floor++){
			for(panel = CALL_UP; panel <=COMMAND; panel++){
				order_table[floor][panel]=0;
			}
		}
	}
	else{
		for(floor=0; floor < N_FLOORS; floor++){
			order_table[floor][panel]=0;
		}
	}
}


void order_reset_current_floor(){
	int floor = elev_get_floor_sensor_signal();
	if(floor!=BETWEEN_FLOORS){
		order_table[floor][COMMAND]=0;
	    if(last_pass_floor_dir == UP) {
		    order_table[floor][CALL_UP]=0;
		    if(!(orders_above(floor))) {
		    	order_table[floor][CALL_DOWN]=0;
		    }
	    }
	    else {
	    	order_table[floor][CALL_DOWN] =0;
	    	if(!(orders_below(floor))) {
	    		order_table[floor][CALL_UP]=0;
	    	}
	    }
	}
}

int orders_above(int c_floor) {
	if(c_floor==N_FLOORS) {
		return 0;
	}
	int i = 0;
	int panel = 0;
	for(i=c_floor+1; i<N_FLOORS; i++) {
		for(panel = CALL_UP; panel <=COMMAND; panel++){
			if(order_table[i][panel]) {
			  return 1;
			}
		}
	}
	return 0;
}

int orders_below(int c_floor) {
	if(c_floor==0) {
		return 0;
	}
	int i = 0;
	int panel = 0;
	for(i=c_floor-1; i>=0; i--) {
		for(panel = CALL_UP; panel <=COMMAND; panel++){
			if(order_table[i][panel]) {
			  return 1;
			}
		}
	}
	return 0;
}

/***************************** Other functions *******************************************/

void order_handle_button_lamps(){
    int floor = 0;
    int button = 0;
	for(floor = 0; floor < N_FLOORS; floor++){
	    for(button = BUTTON_CALL_UP; button<=BUTTON_COMMAND; button++){
			if(!((floor==0 && button==BUTTON_CALL_DOWN) || (floor==N_FLOORS-1 && button==BUTTON_CALL_UP))){ 	//Not considering invalid buttons.
			    if(order_table[floor][button]==0)																//Button and panel is represented by corresponing numbers", so we can take button as input for order_table[][]
			       elev_set_button_lamp(button,floor,0);
			    else    
			       elev_set_button_lamp(button, floor, 1);
		    }		
	    }
    }
}

void order_print(void){							
	printf("|	C_UP	|C_DOWN	|COMMAND|\n");	
	int floor = 0;
	int panel = 0;
	for(floor=N_FLOORS-1; floor>=0;floor--){
		printf("Floor %d ", floor+1);		
		for(panel=CALL_UP; panel<=COMMAND; panel++){
			if(!((floor==0 && panel==CALL_DOWN) || (floor==N_FLOORS-1 && panel==CALL_UP)))
			printf("|%d	", order_table[floor][panel]); 
			else 
			printf("|	");
		}
		printf("| \n");
	}
	printf("\n");
}

/*********************************************************************/

struct node * weightfunction(struct node* root, struct order new_order) {
	int n_elevs = count(root);
	int weight[n_elevs];
	int lowest_weight = 1000;
	int current = 0;
	struct node *iter, *best_elev;
	iter = root;
	int floor_counter;
	int ordered_floor = new_order.floor;
	int current_floor, current_dir;
	while(iter!=0) {
		//Add weight if elevator has order to floors (different weighting for different floors)
		for(floor_counter=0;floor_counter<N_FLOORS; floor_counter++) {
			if(iter->elevinfo.current_orders.panel_cmd[floor_counter]) {
				if(floor_counter == ordered_floor) {
					weight[current]-=10;
				}
				else {
					weight[current]+=3;
				}
			}
			if(floor_counter<N_FLOORS-1 && iter->elevinfo.current_orders.panel_up[floor_counter]) {
				if(floor_counter == ordered_floor) {
					weight[current]-=5;
				}
				else {
					weight[current]+=3;
				}
			}
			if(floor_counter>0 && iter->elevinfo.current_orders.panel_down[floor_counter]) {
				if(floor_counter == ordered_floor) {
					weight[current]-=5;
				}
				else {
					weight[current]+=3;
				}
			}
		}
		//Add weight based on the number of floors the elevator will pass before reaching the ordered floor
		current_floor = iter->elevinfo.current_state.floor;
		current_dir = iter->elevinfo.current_state.direction;
		if(current_dir==DOWN) {
			if(ordered_floor>current_floor) {
				weight[current]+=(3*(current_floor*2+(ordered_floor-current_floor)));
			}
			else if(ordered_floor<current_floor) {
				weight[current]+=(3*(current_floor-ordered_floor));
			}
		}
		else if(current_dir==UP) {
			if(ordered_floor>current_floor) {
				weight[current]+=(3*(ordered_floor-current_floor));
			}
			else if(ordered_floor<current_floor) {
				weight[current]+=(3*(2*(N_FLOORS-current_floor)+(current_floor-ordered_floor)));
			}
		}
		if(weight[current]<lowest_weight){
			lowest_weight = weight[current];
			best_elev = iter;
		}
		current++;
		iter = iter->next;
	}
	return best_elev;
}
