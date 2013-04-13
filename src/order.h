#ifndef _ORDER_H
#define _ORDER_H

#include <arpa/inet.h>
#include "elevator.h"


//Creating different panel types: "ALL" is not a panel-type, but is used if user wants to check all panels.
enum panel_type	 	{CALL_UP, CALL_DOWN, COMMAND, ALL};
enum direction_t 	{UP, DOWN};
enum panel_t 		{PANEL_UP, PANEL_DOWN, PANEL_CMD};

struct order{
	int floor;
	int paneltype;
};

typedef struct {
	int 	active;
	time_t 	timestamp;
} order_t;

struct elevstate{
	int floor;
	int direction;
	int internal_state;
};

struct elevator{
	in_addr_t ip;
	in_addr_t processpair;
	int active;
	struct elevstate current_state;
	order_t current_orders[N_FLOORS][N_PANELS];
};

static struct node {
	struct node *next, *prev;
	struct elevator elevinfo;
};

/* !\brief Order table check functions
 *
 */
int order_check_request_current_floor();
int order_check_request_above();
int order_check_request_below();

/* !\brief Register an new order both local and by broadcasting over network.
 *
 * \param elevator is the elevator you have decided should get the order
 * \param floor is the floor of where the order was made
 * \param panel is the type of order to be executed
 *
 * This function is run when some button is hit
 */
void order_register_new_order(struct node * elevator, int floor, int panel);

/* !\brief Reset orders at current floor. Based on direction of translation
 * Commands are always canceled, call up is canceled if direction is up,
 * call down if direction is down.
 */
void order_reset_current_floor();


void order_flush_panel(struct node * elevator, enum panel_type panel);

/* !\brief Registers an order as done both local and by broadcasting over network.
 *
 * \param floor is the floor of where the order was done
 * \param panel is the type of order executed
 *
 * This function is run when current elevator is done executing an order.
 */
void order_register_as_done(int floor, int panel);

/* !\brief Adds an order local to the node described by elevator.
 *
 */
void order_add_order(struct node * elevator, int floor, int panel);

/* !\brief Returns if there exists requests assigned for this elevator.
 *
 */
int order_requests();

/* !\brief functions returning whether elevator has something undone at current floor,
 * or if it should go some place else.
 *
 */
int order_should_stay();

int order_should_stop();


/* !\brief Routine for monitoring ordered floors.
 *
 * This makes sure no orders are left into infinity. This monitor is also setting button lamps.
 *
 */
void *order_monitor();


/* \!brief Returns the prioritized direction based on orders
 */
int order_pridir();

void order_print_list(order_t orders[][N_PANELS]);

void ordertablemerge(struct node * elevto, struct node * elevfrom, enum panel_type panel);

void clear_order_all_elev(int floor, int panel);


/* List functions for keeping track of elevators */

static int add(struct node * root, struct node * new);

int printlist(struct node * root);

struct node * get(struct node * root, struct node n);

int count(struct node * root);

void activate(struct node * root, struct node n);

void deactivate(struct node * root, struct node n);

struct node * gethead();

struct node * getelevnode(struct elevator elev);

void addelev(struct elevator elev);

void init_order(in_addr_t this_ip);

struct node * weightfunction(struct node* root, struct order new_order);

/* Functions used to sync up new elevators with updated order information. */
void sendsyncinfo(struct node * fromelev, struct node *toelev);

void getsyncinfo(struct node* toelev, struct node * fromelev, int gpdata[], int position, int direction);

/* !\brief Function used to send previous command orders to elevator reconnecting to network.
 *
 */
void recover_elev(struct node * n);


#endif
