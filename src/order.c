#include <stdio.h>
#include "order.h"
#include "elev.h"
#include "communication.h"
#include "network.h"
#include <arpa/inet.h>

static struct node head;


void init_order(in_addr_t this_ip){
	head.elevinfo.ip = this_ip;
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
			order_print_list(iter->elevinfo.current_orders);
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
int rmelev(struct elevator elev){
	struct node n;
	n.elevinfo = elev;
	return rm(&head, n);
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

struct node * getelevnode(struct elevator elev){
	struct node n;
	n.elevinfo = elev;
	return get(&head, n);
}
void addelev(struct elevator elev){
	struct node * n = malloc(sizeof(struct node));
	n->elevinfo = elev;
	add(&head, n);
	printf("Added new elev to order list\n");
	printlist(&head);

}


void addorder(struct node * elevnode, struct order ordr){
		elevnode->elevinfo.current_orders[ordr.floor][ordr.paneltype] = 1;
}


/*****************************          Variables:      **********************************/




//static int head.elevinfo.current_orders[N_FLOORS][N_PANELS];
/*Orders are saved in order_table. There are one column for each panel-type(call_down, call_up and
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
		if(head.elevinfo.current_orders[current_floor][COMMAND] || head.elevinfo.current_orders[current_floor][CALL_UP]) {
			return 1;
		}
	}
	else if(panel==ALL && last_pass_floor_dir==DOWN) {
		if(head.elevinfo.current_orders[current_floor][COMMAND] || head.elevinfo.current_orders[current_floor][CALL_DOWN]) {
						return 1;
		}
	}
	else 
		return(head.elevinfo.current_orders[current_floor][panel]);
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
				if(head.elevinfo.current_orders[floor][panel_counter]==1) return 1;
			}
		}
	}
	else{                                               //Check requests from spesific panel
		for(floor = current_floor+1; floor<N_FLOORS; floor ++){
			if(head.elevinfo.current_orders[floor][panel]==1) return 1;
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
				if(head.elevinfo.current_orders[floor][panel_counter]==1) return 1;
			}
		}	
	}
	else{                                               //Check requests from spesific panel
		for(floor = current_floor-1; floor>=0; floor--){
			if(head.elevinfo.current_orders[floor][panel]==1) return 1;
		}
	}
	return 0;
}

/************************* Actions made on order table: **************************************/
void order_add_order(struct order ord){
	head.elevinfo.current_orders[ord.floor][ord.paneltype] = 1;
}
void order_add() {
	int floor = 0;
	for(floor=0; floor<N_FLOORS; floor++) {
		if(elev_get_button_signal(BUTTON_COMMAND, floor)) {	
			head.elevinfo.current_orders[floor][PANEL_CMD] = 1;
			//head.elevinfo.current_orders[floor][COMMAND]=1;
		}
		if(floor!=N_FLOORS-1 && elev_get_button_signal(BUTTON_CALL_UP, floor)) {	//There are no BUTTON_CALL_UP in the highest floor.	
			//head.elevinfo.current_orders[floor][CALL_UP] = 1;
			int or[FLOORS][N_PANELS];// = {0};
			struct order neworder = {
					.floor = floor,
					.paneltype = PANEL_UP,
			};
			struct node * n = weightfunction(&head, neworder);
			n->elevinfo.current_orders[neworder.floor][neworder.paneltype];
			int gp[] = {floor, PANEL_UP};
			send_msg(OPCODE_NEWORDER, n->elevinfo.ip, or, 0, 0, gp);
		}
		if(floor!=0 && elev_get_button_signal(BUTTON_CALL_DOWN, floor)) {			//There are no BUTTON_CALL_DOWN in the lowest floor.	
			int or[FLOORS][N_PANELS];// = {0};
			struct order neworder = {
					.floor = floor,
					.paneltype = PANEL_UP,
			};
			struct node * n = weightfunction(&head, neworder);
			n->elevinfo.current_orders[neworder.floor][neworder.paneltype];
			//head.elevinfo.current_orders[floor][CALL_DOWN] = 1;
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
				head.elevinfo.current_orders[floor][panel]=0;
			}
		}
	}
	else{
		for(floor=0; floor < N_FLOORS; floor++){
			head.elevinfo.current_orders[floor][panel]=0;
		}
	}
}


void order_reset_current_floor(){
	int floor = elev_get_floor_sensor_signal();
	if(floor!=BETWEEN_FLOORS){
		head.elevinfo.current_orders[floor][COMMAND]=0;
	    if(last_pass_floor_dir == UP) {
		    head.elevinfo.current_orders[floor][CALL_UP]=0;
		    if(!(orders_above(floor))) {
		    	head.elevinfo.current_orders[floor][CALL_DOWN]=0;
		    }
	    }
	    else {
	    	head.elevinfo.current_orders[floor][CALL_DOWN] =0;
	    	if(!(orders_below(floor))) {
	    		head.elevinfo.current_orders[floor][CALL_UP]=0;
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
			if(head.elevinfo.current_orders[i][panel]) {
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
			if(head.elevinfo.current_orders[i][panel]) {
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
			    if(head.elevinfo.current_orders[floor][button]==0)																//Button and panel is represented by corresponding numbers", so we can take button as input for head.elevinfo.current_orders[][]
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
			printf("|%d	", head.elevinfo.current_orders[floor][panel]);
			else 
			printf("|	");
		}
		printf("| \n");
	}
	printf("\n");
}

void order_print_list(int orders[][N_PANELS]){
	printf("|	C_UP	|C_DOWN	|COMMAND|\n");
	int floor = 0;
	int panel = 0;
	for(floor=N_FLOORS-1; floor>=0;floor--){
		printf("Floor %d ", floor+1);
		for(panel=CALL_UP; panel<=COMMAND; panel++){
			if(!((floor==0 && panel==CALL_DOWN) || (floor==N_FLOORS-1 && panel==CALL_UP)))
			printf("|%d	", orders[floor][panel]);
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
			if(iter->elevinfo.current_orders[floor_counter][PANEL_CMD]) {
				if(floor_counter == ordered_floor) {
					weight[current]-=10;
				}
				else {
					weight[current]+=3;
				}
			}
			if(floor_counter<N_FLOORS-1 && iter->elevinfo.current_orders[floor_counter][PANEL_UP]) {
				if(floor_counter == ordered_floor) {
					weight[current]-=5;
				}
				else {
					weight[current]+=3;
				}
			}
			if(floor_counter>0 && iter->elevinfo.current_orders[floor_counter][PANEL_DOWN]) {
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
