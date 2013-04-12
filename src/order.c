#include <stdio.h>
#include "order.h"
#include "elev.h"
#include "communication.h"
#include "network.h"
#include <arpa/inet.h>


static struct node head;
static int last_floor;


struct node * gethead(){
	return &head;
}
int get_last_floor(){
    return last_floor;
}

void set_last_floor(int floor){
    last_floor = floor;
}

void init_order(in_addr_t this_ip){
	head.elevinfo.ip = this_ip;
	head.elevinfo.active = 1;
    head.next 		= 0;
    head.prev		= 0;
	order_flush_panel(&head, CALL_UP);
	order_flush_panel(&head, CALL_DOWN);
	order_flush_panel(&head, COMMAND);
    /* Start thread for monitoring ordered floors and set lamps */
    pthread_t order_monitor_thread;
    if ( (pthread_create(&order_monitor_thread, NULL, order_monitor, (void *) NULL)) < 0){
		perror("err:pthread_create(order_monitor)\n");
	}
}

void order_add_order(struct node * elevator, int floor, int panel){
	elevator->elevinfo.current_orders[floor][panel].active = 1;
	time(&(elevator->elevinfo.current_orders[floor][panel].timestamp));

}

void order_register_new_order(struct node * elevator, int floor, int panel){
	int order[] = {floor, panel};
	send_msg(OPCODE_NEWORDER, elevator->elevinfo.ip, 0, 0, order);
    order_add_order(elevator, floor, panel);
}

void order_register_as_done(int floor, int panel){
    int gp[] = {floor, panel};
    send_msg(OPCODE_ORDERDONE, head.elevinfo.ip , 0, 0, gp);
    head.elevinfo.current_orders[floor][panel].active = 0;
    clear_order_all_elev(floor, panel);
}

void order_flush_panel(struct node * elevator, enum panel_type panel){
	int floor;
	for(floor = 0; floor < N_FLOORS; floor++) {
		elevator->elevinfo.current_orders[floor][panel].active = 0;
		elevator->elevinfo.current_orders[floor][panel].timestamp = 0;
	}
}

void order_reset_current_floor(){
	int floor = control_getcurpos();
	if(floor!=BETWEEN_FLOORS){
		if ((floor == 0) && head.elevinfo.current_orders[floor][CALL_UP].active) order_register_as_done(floor, CALL_UP);
		if ((floor == N_FLOORS-1) && head.elevinfo.current_orders[floor][CALL_DOWN].active) order_register_as_done(floor, CALL_DOWN);
		if(head.elevinfo.current_orders[floor][COMMAND].active)	order_register_as_done(floor, COMMAND);
		else{
			if(head.elevinfo.current_orders[floor][CALL_UP].active!=head.elevinfo.current_orders[floor][CALL_DOWN].active ){
				if(head.elevinfo.current_orders[floor][CALL_UP].active ){
					order_register_as_done(floor, CALL_UP);
					set_last_dir(UP);
				}
				else{
					order_register_as_done(floor, CALL_DOWN);
					set_last_dir(DOWN);
				}
			}
		}
		if((get_last_dir() == UP) && head.elevinfo.current_orders[floor][CALL_UP].active ) order_register_as_done(floor, CALL_UP);
		else if((get_last_dir() == DOWN) && head.elevinfo.current_orders[floor][CALL_DOWN].active ) order_register_as_done( floor, CALL_DOWN);
	}

}

void clear_order_all_elev(int floor, int panel){
	struct node * iter = gethead();
	if(panel==COMMAND) iter->elevinfo.current_orders[floor][panel].active = 0;
	else {
		while(iter!=0){
				iter->elevinfo.current_orders[floor][panel].active = 0;
				iter = iter->next;
		}
	}
}

int order_check_request_current_floor(){
	int current_floor = control_getcurpos();
	if(current_floor==BETWEEN_FLOORS)
	    return 0;

	if(get_last_dir()==UP){
		return(head.elevinfo.current_orders[current_floor][COMMAND].active || head.elevinfo.current_orders[current_floor][CALL_UP].active);
	}
	else if(get_last_dir()==DOWN) {
		return(head.elevinfo.current_orders[current_floor][COMMAND].active || head.elevinfo.current_orders[current_floor][CALL_DOWN].active);
	}
	return 0;
}

int order_check_request_above(){
	int current_floor = control_getcurpos();
	if(current_floor==BETWEEN_FLOORS){
	    if(get_last_dir()==UP) current_floor = last_floor;	/* Means elevator is probably between last_floor and last_floor+1*
	    													 * Then checks are to be done relative to floor recently passed. */
	    else current_floor = last_floor-1;					/* Else check relative to the floor below elevator.*/
	}
	else last_floor = current_floor;
	int floor = 0;
	if(current_floor == N_FLOORS-1)	return 0;
	int panel_counter = 0;
	for(panel_counter = CALL_UP; panel_counter<=COMMAND; panel_counter++){
		for(floor = current_floor+1; floor<N_FLOORS; floor ++){
			if(head.elevinfo.current_orders[floor][panel_counter].active==1) return 1;
		}
	}
	return 0;
}

int order_check_request_below(){
	int current_floor = control_getcurpos();
	if(current_floor==BETWEEN_FLOORS){
	    if(get_last_dir()==UP) current_floor = last_floor+1;  	/* Then the checks are to be done relative to floor above elevator.	*/
	    else current_floor = last_floor;     					/* Else check requests relative to the last floor passed.			*/
	}
	if(current_floor == 0) return 0;
	int floor, panel_counter;
	for(panel_counter = CALL_UP; panel_counter<=COMMAND; panel_counter++){
		for(floor = current_floor-1; floor>=0; floor--){
			if(head.elevinfo.current_orders[floor][panel_counter].active==1) return 1;
		}
	}
	return 0;
}


//Functions used as guards by the state machine

int order_requests(){
	int floor, panel;
	struct node * head = gethead();
	for (panel = CALL_UP; panel<=COMMAND; panel++){
		for(floor = 0; floor< N_FLOORS; floor ++){
			if(head->elevinfo.current_orders[floor][panel].active){
				return 1;
			}
		}
	}
	return 0;
}

int order_should_stay(){
	int current_floor = control_getcurpos();
	if(current_floor==BETWEEN_FLOORS){
		return 0;
	}
	struct node * head = gethead();
	if(get_last_dir()==UP){
		if((head->elevinfo.current_orders[current_floor][COMMAND].active==1) || (head->elevinfo.current_orders[current_floor][CALL_UP].active==1)) {
			return 1;
		}
		else if(!order_check_request_above()){
			return 1;
		}
	}
	else if(get_last_dir()==DOWN) {
		if((head->elevinfo.current_orders[current_floor][COMMAND].active==1) || (head->elevinfo.current_orders[current_floor][CALL_DOWN].active==1)) {
			return 1;
		}
		else if(!order_check_request_below()){
			return 1;
		}
	}
	return 0;
}

int order_should_stop(){
	int current_floor = control_getcurpos();
	if(current_floor==BETWEEN_FLOORS){
		return 0;
	}
	if(current_floor==N_FLOORS-1 || current_floor==0) {
		return 1;
	}
	struct node * head = gethead();
	if(get_last_dir()==UP){
		if((head->elevinfo.current_orders[current_floor][COMMAND].active==1) || (head->elevinfo.current_orders[current_floor][CALL_UP].active==1)) {
			return 1;
		}
		else if(!order_check_request_above()){
			return 1;
		}
	}
	else if(get_last_dir()==DOWN) {
		if((head->elevinfo.current_orders[current_floor][COMMAND].active==1) || (head->elevinfo.current_orders[current_floor][CALL_DOWN].active==1)) {
			return 1;
		}
		else if(!order_check_request_below()){
			return 1;
		}
	}
	return 0;
}

int order_pridir(){
	//Check if there is a request at current floor in right direction.
	if(order_check_request_current_floor()) {
		return 0;
	}

	//Check if there is a request above
	else if(order_check_request_above() && (get_last_dir()==UP)){
		return 1;
	}
	//Check if there is a request below
	else if(order_check_request_below() && (get_last_dir()==DOWN)) {
		return -1;
	}
	//Check if there is a request above when the direction is DOWN. This is served if there are no requests below.
	else if(order_check_request_above() && !(order_check_request_below())) {
		return 1;
	}
	//Check if there is a request below when the direction is UP. This is served if there are no requests above.
	else if(order_check_request_below() && !(order_check_request_above())) {
		return -1;
	}
	else{
		return 0;
	}
}


void ordertablemerge(struct node * elevto, struct node * elevfrom, enum panel_type panel){
	int floor;
	if(panel == ALL){
		for(floor=0; floor < N_FLOORS; floor++){
			for(panel = CALL_UP; panel <=COMMAND; panel++){
				if(elevfrom->elevinfo.current_orders[floor][panel].active){
					order_add_order(elevto, floor, panel);
				}
			}
		}
	}
	else{
		for(floor=0; floor < N_FLOORS; floor++){
			if(elevfrom->elevinfo.current_orders[floor][panel].active){
				order_add_order(elevto, floor, panel);
			}

		}
	}
}

void *order_monitor(){
    int floor, panel;
    struct node * iter = &head;
    time_t current_time;
    int anyorder = 0;
    while(1){
    	time(&current_time);

        /* Loop command orders on local elevator */
        panel = COMMAND;
        for(floor = 0; floor < N_FLOORS; floor++){
            if(iter->elevinfo.current_orders[floor][panel].active) elev_set_button_lamp(panel,floor,1);
            else elev_set_button_lamp(panel, floor, 0);
        }
        /* Loop call up and call down panel on all elevators */
		for(floor = 0; floor < N_FLOORS; floor++){
			for(panel = CALL_UP; panel<COMMAND; panel++){
				if(!((floor==0 && panel==CALL_DOWN) || (floor==N_FLOORS-1 && panel==CALL_UP))){ 	/* Not considering invalid buttons.*/
					while(iter!=0){
						if(iter->elevinfo.active){
							if(iter->elevinfo.current_orders[floor][panel].active){
								/* Check timestamp - if timeout, then take over order */
								if(current_time - iter->elevinfo.current_orders[floor][panel].timestamp > MAXWAIT_ORDER_TIME){
									order_register_new_order(&head, floor, panel);
								}
								anyorder = 1;
								break;
							}
						}
						iter = iter->next;
					}
					if(anyorder) elev_set_button_lamp(panel,floor,1);
					else elev_set_button_lamp(panel, floor, 0);
					anyorder = 0;
				}
				/* Restart scan */
				iter = &head;
			}
		}
    }
}

void order_print_list(order_t orders[][N_PANELS]){
	printf("|	C_UP	|C_DOWN	|COMMAND|\n");
	int floor = 0;
	int panel = 0;
	for(floor=N_FLOORS-1; floor>=0;floor--){
		printf("Floor %d ", floor+1);
		for(panel=CALL_UP; panel<=COMMAND; panel++){
			if(!((floor==0 && panel==CALL_DOWN) || (floor==N_FLOORS-1 && panel==CALL_UP)))
			printf("|%d	", orders[floor][panel].active);
			else
			printf("|	");
		}
		printf("| \n");
	}
	printf("\n");
}

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
				if(iter->elevinfo.current_orders[floor_counter][PANEL_CMD].active) {
					if(floor_counter == ordered_floor) {
						weight[current]-=10;
					}
					else {
						weight[current]+=3;
					}
				}
				if(floor_counter<N_FLOORS-1 && iter->elevinfo.current_orders[floor_counter][PANEL_UP].active) {
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
				if(floor_counter>0 && iter->elevinfo.current_orders[floor_counter][PANEL_DOWN].active) {
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
			if(weight[current]<lowest_weight){
				lowest_weight = weight[current];
				best_elev = iter;
			}
		}
		else{
			;
		}
		current++;
		iter = iter->next;
	}
	return best_elev;
}


/* Linked list for keeping track of connected elevators *
 * 														*/

/* !\brief Add new elevator to list over available order takers
 *
 */
static int add(struct node * root, struct node * new){
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

	order_flush_panel(new, CALL_UP);
	order_flush_panel(new, CALL_DOWN);
	order_flush_panel(new, COMMAND);

	return 1;

}

void activate(struct node * root, struct node n){
	struct node * np = get(root, n);
	np->elevinfo.active = 1;
}

void deactivate(struct node * root, struct node n){
	struct node * np = get(root, n);
	np->elevinfo.active = 0;
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

int count(struct node * root){
	struct node * iter = root;
	int count = 0;
	while(iter!=0){
		count ++;
		iter = iter->next;
	}
	return count;
}

struct node * getelevnode(struct elevator elev){
	struct node n;
	n.elevinfo = elev;
	return get(&head, n);
}

void addelev(struct elevator elev){
	struct node * n = malloc(sizeof(struct node));
	n->elevinfo = elev;
	add(&head, n);

}
//
//void weightfunction_sim() {
//
//	//Init elevator
//	init_order(inet_addr(getlocalip()));
//	gethead()->elevinfo.ip = 1;
//	gethead()->elevinfo.current_state.floor=3;
//	gethead()->elevinfo.current_state.direction=DOWN;
//	struct elevator otherelev;
//	otherelev.ip = 2;
//	addelev(otherelev);
//
//	//Add orders
//
//	int str = 5;
//	struct order orders[str];
//
//	orders[5].floor = 1;
//	orders[5].paneltype = CALL_UP;
//	orders[0].floor = 1;
//	orders[0].paneltype = CALL_DOWN;
//	orders[2].floor = 3;
//	orders[2].paneltype = CALL_DOWN;
//	orders[4].floor = 2;
//	orders[4].paneltype = CALL_DOWN;
//	orders[3].floor = 2;
//	orders[3].paneltype = CALL_UP;
//	orders[1].floor = 0;
//	orders[1].paneltype = CALL_UP;
//
//
//	struct node * n;
//	int i;
//	for(i=0; i<str; i++) {
//		if(orders[i].paneltype == COMMAND){
//			n = gethead();
//		}
//		else{
//			n = weightfunction(gethead(), orders[i]);
//		}
//		int order[] = {orders[i].floor, orders[i].paneltype};
//		n->elevinfo.current_orders[orders[i].floor][orders[i].paneltype].active = 1;
//	}
//
////	printlist(gethead());
//}
