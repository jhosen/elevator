#include <stdio.h>
#include "order.h"
#include "elev.h"
#include "communication.h"
#include "network.h"
#include <arpa/inet.h>

static struct node head;

struct node * gethead(){
	return &head;
}

void init_order(in_addr_t this_ip){
	head.elevinfo.ip = this_ip;
	head.elevinfo.active = 1;
    head.next 		= 0;
    head.prev		= 0;

    /* Start thread for monitoring ordered floors and set lamps */
    pthread_t lamp_monitor;
    if ( (pthread_create(&lamp_monitor, NULL, order_handle_button_lamps, (void *) NULL)) < 0){
		perror("err:pthread_create(lamp_monitor)\n");
	}
}

/** Linked list for keeping track of connected elevators **/


/*void initlist(struct node * root){
	root = malloc( sizeof(struct node) );
    root->next 		= 0;
    root->prev		= 0;
}*/
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
	iter->elevinfo.active = 1;

	//Used in simulation of weightfunction
//	iter->elevinfo.current_state.floor=0;
//	iter->elevinfo.current_state.direction=UP;

	cleartable(iter->elevinfo.current_orders);
	return 1; // success
    
}

void cleartable(int table[][N_PANELS]){
	int floor, panel;
	for(floor = 0; floor < N_FLOORS; floor ++){
		for(panel = 0; panel<N_PANELS; panel ++){
			table[floor][panel] = 0;
		}
	}
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

void activate(struct node * root, struct node n){
	struct node * np = get(root, n);
	np->elevinfo.active = 1;
}

void deactivate(struct node * root, struct node n){
	struct node * np = get(root, n);
	np->elevinfo.active = 0;
}

/* ¤%%¤¤%¤%¤%¤%¤%¤%¤%¤%¤%¤%¤%¤%¤%
struct node * gethighestactiveelevnode(){
	struct node * iter = root;
	while(iter!=0){
		if(iter->elevinfo.active){
			if((iter->elevinfo.ip) == n.elevinfo.ip){
				return iter;
			}
		}
		iter = iter->next;
	}
}
void deactivateelev(struct elevator elev){
	struct node * n = getelevnode(elev);
	n->elevinfo.active = 0;
}*/
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

int count(struct node * root){
	struct node * iter = root;
	int count = 0;
	while(iter!=0){
		count ++;
		iter = iter->next;
	}
	return count;
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
	//printf("Added new elev to order list\n");
//	printlist(&head);

}


void addorder(struct node * elevnode, struct order ordr){
		elevnode->elevinfo.current_orders[ordr.floor][ordr.paneltype] = 1;
}


void ordertablemerge(int ordto[][N_PANELS], int ordfrom[][N_PANELS], enum panel_type panel){
	int floor;
	if(panel == ALL){
		for(floor=0; floor < N_FLOORS; floor++){
			for(panel = CALL_UP; panel <=COMMAND; panel++){
				ordto[floor][panel] |= ordfrom[floor][panel];
			}
		}
	}
	else{
		for(floor=0; floor < N_FLOORS; floor++){
			ordto[floor][panel] |= ordfrom[floor][panel];
		}
	}
}

/*****************************          Variables:      **********************************/



/*Orders are saved in order_table. There are one column for each panel-type(call_down, call_up and
                                                command). As an example order_table[2][CALL_UP] refers to orders made from call_up-panel in 
                                                3rd floor.*/ 
//static enum direction_t last_pass_floor_dir;	// last_pass_floor_dir keeps track of the position of the elevator by remembering the direction the elevator went when passing a floor.
static int last_floor;							// Used to distinct a new floor from last floor.

/***************************** Get/set functions: **********************************/

//void set_last_pass_floor_dir(enum direction_t dir) {
//	last_pass_floor_dir = dir;
//}

int get_last_floor(){
    return last_floor;
}

void set_last_floor(int floor){
    last_floor = floor;
}

/******************************* Check order table functions ******************************************/

int order_check_request_current_floor(){
	int current_floor = getcurrentpos();
	if(current_floor==BETWEEN_FLOORS)
	    return 0;

	if(get_last_dir()==UP){
		return(head.elevinfo.current_orders[current_floor][COMMAND] || head.elevinfo.current_orders[current_floor][CALL_UP]);
	}
	else if(get_last_dir()==DOWN) {
		return(head.elevinfo.current_orders[current_floor][COMMAND] || head.elevinfo.current_orders[current_floor][CALL_DOWN]);
	}
	return 0;
}

int order_check_request_above(){
	int current_floor = getcurrentpos();
	printf("current floor %i\n", current_floor);
	if(current_floor==BETWEEN_FLOORS){
	    if(get_last_dir()==UP){                    		//Means elevator is probably between last_floor and last_floor+1
	        current_floor = last_floor;					//Then checks are to be done relative to floor recently passed.
	    }
	    else{
	        current_floor = last_floor-1;				//Else check relative to the floor below elevator.
	    }
	} 

	int floor = 0;
	int panel_counter = 0;
	for(panel_counter = CALL_UP; panel_counter<=COMMAND; panel_counter++){
		for(floor = current_floor+1; floor<N_FLOORS; floor ++){
			if(head.elevinfo.current_orders[floor][panel_counter]==1) return 1;
		}
	}
	return 0;
}

int order_check_request_below(){
	int current_floor = getcurrentpos();
	if(current_floor==BETWEEN_FLOORS){
	    if(get_last_dir()==UP){							//Means elevator is probably between last_floor-1 and last_floor
	        current_floor = last_floor+1;   			//Then the checks are to be done relative to floor above elevator.
	    }
	    else{                               
	        current_floor = last_floor;     			//Else check requests relative to the last floor passed.
	    }
	}
	if(current_floor == 0) {
			return 0;
	}
	int floor = 0;
	int panel_counter = 0;
	for(panel_counter = CALL_UP; panel_counter<=COMMAND; panel_counter++){
		for(floor = current_floor-1; floor>=0; floor--){
			if(head.elevinfo.current_orders[floor][panel_counter]==1) return 1;
		}	
	}
	return 0;
}

/************************* Actions made on order table: **************************************/
void order_add_order(struct order ord){
	head.elevinfo.current_orders[ord.floor][ord.paneltype] = 1;
}

/*
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
}*/
void order_flush_panel(struct node * elevator, enum panel_type panel){
	int floor;
	for(floor = 0; floor < FLOORS; floor++) {
		elevator->elevinfo.current_orders[floor][panel] = 0;
	}
}

void order_reset_current_floor(){
	int floor = getcurrentpos();
	if(floor!=BETWEEN_FLOORS){
		if ((floor == 0) && head.elevinfo.current_orders[floor][CALL_UP]){
					order_register_as_done(floor, CALL_UP);
		}
		if ((floor == N_FLOORS-1) && head.elevinfo.current_orders[floor][CALL_DOWN]){
					order_register_as_done(floor, CALL_DOWN);
		}
		if(head.elevinfo.current_orders[floor][COMMAND]){
			order_register_as_done(floor, COMMAND);
		}
		else{
			if(head.elevinfo.current_orders[floor][CALL_UP]!=head.elevinfo.current_orders[floor][CALL_DOWN] ){
				if(head.elevinfo.current_orders[floor][CALL_UP] ){
					order_register_as_done(floor, CALL_UP);
					set_last_dir(UP);
				}
				else{
					order_register_as_done(floor, CALL_DOWN);
					set_last_dir(DOWN);
				}
			}
		}
//        if(head.elevinfo.current_orders[floor][CALL_UP]!=head.elevinfo.current_orders[floor][CALL_DOWN] ){
//			if(head.elevinfo.current_orders[floor][CALL_UP] ){
//                order_register_as_done(floor, CALL_UP);
//                set_last_dir(UP);
//			}
//			else{
//                order_register_as_done(floor, CALL_DOWN);
//                set_last_dir(DOWN);
//			}
//		}
		if((get_last_dir() == UP) && head.elevinfo.current_orders[floor][CALL_UP] ) {
		    order_register_as_done(floor, CALL_UP);
	    }
		else if((get_last_dir() == DOWN) && head.elevinfo.current_orders[floor][CALL_DOWN] ){
            order_register_as_done( floor, CALL_DOWN);
	    }
	}

}

void order_register_new_order(struct node * elevator, int floor, int panel){
	int order[] = {floor, panel};
	int ordummy[FLOORS][N_PANELS];
	send_msg(OPCODE_NEWORDER, elevator->elevinfo.ip, ordummy, 0, 0, order);
    elevator->elevinfo.current_orders[floor][panel] = 1;
}


void order_register_as_done(int floor, int panel){
    int gp[] = {floor, panel};
    int ordummy[FLOORS][N_PANELS];
    send_msg(OPCODE_ORDERDONE, head.elevinfo.ip, ordummy, 0, 0, gp);
    head.elevinfo.current_orders[floor][panel] = 0;
    clear_order_all_elev(floor, panel);
//    printf("Order done for floor %i\n and panel %i\n", floor, panel);
}

/***************************** Other functions *******************************************/
void clear_order_all_elev(int floor, int panel){
	struct node * iter = gethead();
	while(iter!=0){
			iter->elevinfo.current_orders[floor][panel] = 0;
			iter = iter->next;
	}
}

/* !\brief Routine for monitoring ordered floors for setting button lamps.
 *
 */
void *order_handle_button_lamps(){
    int floor, panel;
    struct node * iter = &head;
    int anyorder = 0;
    while(1){

        /* Loop command orders on local elevator */
        panel = COMMAND;
        for(floor = 0; floor < FLOORS; floor++){
            if(iter->elevinfo.current_orders[floor][panel]==1)
                elev_set_button_lamp(panel,floor,1);
            else
                elev_set_button_lamp(panel, floor, 0);
        }

        /* Loop call up and call down */

		for(floor = 0; floor < FLOORS; floor++){
			for(panel = CALL_UP; panel<COMMAND; panel++){
				if(!((floor==0 && panel==CALL_DOWN) || (floor==N_FLOORS-1 && panel==CALL_UP))){ 	//Not considering invalid buttons.
					while(iter!=0){
						if(iter->elevinfo.active){
							if(iter->elevinfo.current_orders[floor][panel]){
								anyorder = 1;
								break;
							}
						}
						iter = iter->next;
					}
					if(anyorder){
						elev_set_button_lamp(panel,floor,1);
					}
					else{
						elev_set_button_lamp(panel, floor, 0);
					}
					anyorder = 0;
				}
				/* Restart scan */
				iter = &head;
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
	int lowest_weight = 500;
	int current = 0;
	struct node *iter, *best_elev;
	iter = root;
	best_elev = root;
	int floor_counter;
	int ordered_floor = new_order.floor;
	int current_floor, current_dir;
	while(iter!=0) {
		if(iter->elevinfo.active){
			current_floor = iter->elevinfo.current_state.floor;
			current_dir = iter->elevinfo.current_state.direction;
			weight[current] = 0;
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
					if(floor_counter == ordered_floor && current_dir == UP && current_floor>ordered_floor) {
						weight[current]-=10;
					}
					else if(floor_counter == ordered_floor && ((current_dir == DOWN) || (current_dir == UP && current_floor>ordered_floor))) {
						weight[current]+=15;
					}
					else {
						weight[current]+=3;
					}
				}
				if(floor_counter>0 && iter->elevinfo.current_orders[floor_counter][PANEL_DOWN]) {
					if(floor_counter == ordered_floor && current_dir == DOWN && current_floor<ordered_floor) {
						weight[current]-=10;
					}
					else if(floor_counter == ordered_floor && (current_dir == UP || (current_dir == DOWN && current_floor<ordered_floor))) {
						weight[current]+=15;
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
					weight[current]+=(3*((N_FLOORS-current_floor)*2+(current_floor-ordered_floor)));
				}
			}
			printf("Elevator %i has weight: %i\n", iter->elevinfo.ip, weight[current]);
			if(weight[current]<lowest_weight){
				lowest_weight = weight[current];
				best_elev = iter;
			}
		}
		else{
//			printf("Elevator not active.\n");
			;
		}
		current++;
		iter = iter->next;
	}
	printf("Best elevator : %i\n", best_elev->elevinfo.ip);
	//order_print();
	return best_elev;
}


void weightfunction_sim() {

	//Init elevator
	init_order(inet_addr(getlocalip()));
	gethead()->elevinfo.ip = 1;
	gethead()->elevinfo.current_state.floor=3;
	gethead()->elevinfo.current_state.direction=DOWN;
	struct elevator otherelev;
	otherelev.ip = 2;
	addelev(otherelev);

	//Add orders

	int str = 5;
	struct order orders[str];

	orders[5].floor = 1;
	orders[5].paneltype = CALL_UP;
	orders[0].floor = 1;
	orders[0].paneltype = CALL_DOWN;
	orders[2].floor = 3;
	orders[2].paneltype = CALL_DOWN;
	orders[4].floor = 2;
	orders[4].paneltype = CALL_DOWN;
	orders[3].floor = 2;
	orders[3].paneltype = CALL_UP;
	orders[1].floor = 0;
	orders[1].paneltype = CALL_UP;


	struct node * n;
	int i;
	for(i=0; i<str; i++) {
		if(orders[i].paneltype == COMMAND){
			n = gethead();
		}
		else{
			n = weightfunction(gethead(), orders[i]);
		}
		int order[] = {orders[i].floor, orders[i].paneltype};
		n->elevinfo.current_orders[orders[i].floor][orders[i].paneltype] = 1;
	}

	printlist(gethead());
}
