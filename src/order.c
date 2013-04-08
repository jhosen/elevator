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
	iter->elevinfo.active = 1;
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

/*
void activateelev(struct elevator elev){
	struct node * n = getelevnode(elev);
	n->elevinfo.active = 1;
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

int should_stop(){
	enum panel_type panel = ALL;
	int current_floor = getcurrentpos();
	if(current_floor==BETWEEN_FLOORS) 
	    return 0;
	
	/* Wants to check all panels: */
	if(panel==ALL && get_last_dir()==UP){
		if(head.elevinfo.current_orders[current_floor][COMMAND] || head.elevinfo.current_orders[current_floor][CALL_UP]) {
			return 1;
		}
		else if(!order_check_request_above(ALL))
			return 1;
	}
	else if(panel==ALL && get_last_dir()==DOWN) {
		if(head.elevinfo.current_orders[current_floor][COMMAND] || head.elevinfo.current_orders[current_floor][CALL_DOWN]) {
						return 1;
		}
		else if(!order_check_request_below(ALL))
			return 1;
	}
	else
		return(head.elevinfo.current_orders[current_floor][panel]);
	return 0;
}

int order_check_request_current_floor(enum panel_type panel){
	int current_floor = getcurrentpos();
	if(current_floor==BETWEEN_FLOORS)
	    return 0;

	/* Wants to check all panels: */
	if(panel==ALL && get_last_dir()==UP){
		if(head.elevinfo.current_orders[current_floor][COMMAND] || head.elevinfo.current_orders[current_floor][CALL_UP]) {
			return 1;
		}
	}
	else if(panel==ALL && get_last_dir()==DOWN) {
		if(head.elevinfo.current_orders[current_floor][COMMAND] || head.elevinfo.current_orders[current_floor][CALL_DOWN]) {
						return 1;
		}
	}
	else 
		return(head.elevinfo.current_orders[current_floor][panel]);
	return 0;
}

int order_check_request_above(enum panel_type panel){
	int current_floor = getcurrentpos();
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
	int current_floor = getcurrentpos();
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


void order_reset_current_floor(){
	int floor = getcurrentpos();
	if(floor!=BETWEEN_FLOORS){
		head.elevinfo.current_orders[floor][COMMAND]=0;
		// SEND COMMAND DONE
		int gp[] = {floor, PANEL_CMD};
		int or[FLOORS][N_PANELS];
		send_msg(OPCODE_ORDERDONE, head.elevinfo.ip, or, 0, 0, gp);
		//elev_set_button_lamp(ELEV_DIR_COMMAND, floor, 0);

		if(head.elevinfo.current_orders[floor][CALL_UP]!=head.elevinfo.current_orders[floor][CALL_DOWN] ){
			if(head.elevinfo.current_orders[floor][CALL_UP] ){
				head.elevinfo.current_orders[floor][CALL_UP] = 0;
				int gp[] = {floor, CALL_UP};
				int or[FLOORS][N_PANELS];
				send_msg(OPCODE_ORDERDONE, head.elevinfo.ip, or, 0, 0, gp);
			}
			else{
				head.elevinfo.current_orders[floor][CALL_DOWN] = 0;
				int gp[] = {floor, CALL_DOWN};
				int or[FLOORS][N_PANELS];
				send_msg(OPCODE_ORDERDONE, head.elevinfo.ip, or, 0, 0, gp);
			}
		}
		else if(get_last_dir() == UP) {
		    head.elevinfo.current_orders[floor][CALL_UP]=0;
		    // SEND CALLUP
			int gp[] = {floor, PANEL_UP};
			int or[FLOORS][N_PANELS];
			send_msg(OPCODE_ORDERDONE, head.elevinfo.ip, or, 0, 0, gp);
			//elev_set_button_lamp(ELEV_DIR_UP, floor, 0);


//		    if(!(orders_above(floor))) {
//		    	head.elevinfo.current_orders[floor][CALL_DOWN]=0;
//		    	// SEND CALLDWN
//				int gp[] = {floor, PANEL_DOWN};
//				int or[FLOORS][N_PANELS];
//				send_msg(OPCODE_ORDERDONE, head.elevinfo.ip, or, 0, 0, gp);
//				elev_set_button_lamp(ELEV_DIR_DOWN, floor, 0);
//
//		    }
	    }
	    else {
	    	head.elevinfo.current_orders[floor][CALL_DOWN] =0;
	    	// SEND CALLDWN
			int gp[] = {floor, PANEL_DOWN};
			int or[FLOORS][N_PANELS];
			send_msg(OPCODE_ORDERDONE, head.elevinfo.ip, or, 0, 0, gp);
			//elev_set_button_lamp(ELEV_DIR_DOWN, floor, 0);
//	    	if(!(orders_below(floor))) {
//	    		head.elevinfo.current_orders[floor][CALL_UP]=0;
//	    		//SENDCALLUP
//	    		int gp[] = {floor, PANEL_UP};
//	    		int or[FLOORS][N_PANELS];
//	    		send_msg(OPCODE_ORDERDONE, head.elevinfo.ip, or, 0, 0, gp);
//	    		elev_set_button_lamp(ELEV_DIR_UP, floor, 0);
//	    	}
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


/* !\brief Routine for monitoring ordered floors for setting button lamps.
 *
 */
void *order_handle_button_lamps(){
    int floor, panel;
    struct node * iter = &head;

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
        while(iter!=0){
            for(floor = 0; floor < FLOORS; floor++){
                for(panel = CALL_UP; panel<COMMAND; panel++){
                    if(!((floor==0 && panel==CALL_DOWN) || (floor==N_FLOORS-1 && panel==CALL_UP))){ 	//Not considering invalid buttons.
                        if(iter->elevinfo.current_orders[floor][panel]==1){
                            elev_set_button_lamp(panel,floor,1);
                        }
                        else
                            elev_set_button_lamp(panel, floor, 0);
                    }
                }
            }
            iter = iter->next;
        }

        /* Restart scan */
        iter = &head;
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
			weight[current] = 0;
			printf("elev %i -weight:%i\n", iter->elevinfo.ip, weight[current]);
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
			printf("elev %i -weight:%i\n", iter->elevinfo.ip, weight[current]);
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
				printf("elev %i -weight:%i\n", iter->elevinfo.ip, weight[current]);
			}
			else if(current_dir==UP) {
				if(ordered_floor>current_floor) {
					weight[current]+=(3*(ordered_floor-current_floor));
				}
				else if(ordered_floor<current_floor) {
					weight[current]+=(3*(2*(N_FLOORS-current_floor)+(current_floor-ordered_floor)));
				}
			}
			printf("elev %i - weight: %i\n", iter->elevinfo.ip, weight[current]);
			if(weight[current]<lowest_weight){
				lowest_weight = weight[current];
				best_elev = iter;
				printf("NEW BESTelev %i -weight:%i\n", iter->elevinfo.ip, weight[current]);
			}
			printf("elev %i -weight:%i\n", iter->elevinfo.ip, weight[current]);
		}
		else{
			printf("Elev not active.\n");

		}
		current++;
		iter = iter->next;
	}
	printf("best elev : %i\n", best_elev->elevinfo.ip);
	order_print();
	return best_elev;
}
