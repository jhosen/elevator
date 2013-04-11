#ifndef _ORDER_H
#define _ORDER_H

#include <arpa/inet.h>

#define N_PANELS 3
#define BETWEEN_FLOORS -1

/* Linked list for keeping track of all elevators online */
#define FLOORS 4
#define N_PANELS 3


struct order{
	int floor;
	int paneltype;
};

struct elevstate{
	int floor;
	int direction;
	int internal_state;
};

struct elevator{
	in_addr_t ip;
	int active;
	struct elevstate current_state;
	int current_orders[FLOORS][N_PANELS];
};

static struct node {
	struct node *next, *prev;
	struct elevator elevinfo;
};

//Creating different panel types: "ALL" is not a panel-type, but is used if user wants to check all panels.
enum panel_type {CALL_UP, CALL_DOWN, COMMAND, ALL};
enum direction_t {UP, DOWN};

/*********************************************** GET/SET FUNCTIONS ***********************************************/
int get_last_floor();
void set_last_floor(int floor);
void set_last_pass_floor_dir(enum direction_t dir);

/*************************************************** Functions ***********************************************/

// Checks if there are orders at, above or below elevator. When argument is, all panels are checked.
int order_check_request_current_floor();
int order_check_request_above();
int order_check_request_below();
                                            		   
void order_empty(enum panel_type panel);

/* !\brief Register an new order both local and by broadcasting over network.
 *
 * \param elevator is the elevator you have decided should get the order
 * \param floor is the floor of where the order was made
 * \param panel is the type of order to be executed
 *
 * This function is run when some button is hit
 */
void order_register_new_order(struct node * elevator, int floor, int panel);

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
                     
void order_add();

//Handle_button_lamps control all panel lights. 									
void *order_handle_button_lamps();

// Function available for service engineers for testing.
void order_print(void);

void order_print_list(int orders[][N_PANELS]);

void ordertablemerge(int ordto[][N_PANELS], int ordfrom[][N_PANELS], enum panel_type panel);



/* List functions for keeping track of elevators */

static void initlist(struct node * root);

static int printlist(struct node * root);

static int add(struct node * root, struct node * new);

static int rm(struct node* root, struct node n);

static int find(struct node * root, struct node n);

int count(struct node * root);

struct node * get(struct node * root, struct node n);

void activate(struct node * root, struct node n);

void deactivate(struct node * root, struct node n);

void activateelev(struct elevator elev);

void deactivateelev(struct elevator elev);

void order_add_order(struct order ord);

//void cleartable(int table[][N_PANELS]);
void cleartable(int * table, int nrows, int ncols);

void clear_order_all_elev(int floor, int panel);

#define PANEL_CMD 2
#define PANEL_UP 0
#define PANEL_DOWN 1

struct node * gethead();

struct node * getelevnode(struct elevator elev);

void addelev(struct elevator elev);

int rmelev(struct elevator elev);

void addorder(struct node * elevnode, struct order ordr);

void init_order(in_addr_t this_ip);

struct node * weightfunction(struct node* root, struct order new_order);

#endif
