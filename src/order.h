#ifndef _ORDER_H
#define _ORDER_H

#include <arpa/inet.h>

#define N_PANELS 3
#define BETWEEN_FLOORS -1


//Creating different panel types: "ALL" is not a panel-type, but is used if user wants to check all panels.
enum panel_type {CALL_UP, CALL_DOWN, COMMAND, ALL};
enum direction_t {UP, DOWN};

/*********************************************** GET/SET FUNCTIONS ***********************************************/
int get_last_floor();
void set_last_floor(int floor);
void set_last_pass_floor_dir(enum direction_t dir);

/*************************************************** Functions ***********************************************/

// Checks if there are orders at, above or below elevator. When argument is, all panels are checked.
int order_check_request_current_floor(enum panel_type panel);
int order_check_request_above(enum panel_type panel);
int order_check_request_below(enum panel_type panel);
                                            		   
void order_empty(enum panel_type panel);
                                                        
void order_reset_current_floor();
                     
void order_add();

int orders_above(int current_floor);
int orders_below(int current_floor);

int should_stop();

//Handle_button_lamps control all panel lights. 									
void order_handle_button_lamps();

// Function available for service engineers for testing.
void order_print(void);

void order_print_list(int orders[][N_PANELS]);

void ordertablemerge(int ordto[][N_PANELS], int ordfrom[][N_PANELS], enum panel_type panel);

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


/* List functions for keeping track of elevators */

static void initlist(struct node * root);

static int printlist(struct node * root);

static int count(struct node * root);

static int add(struct node * root, struct node * new);

static int rm(struct node* root, struct node n);

static int find(struct node * root, struct node n);

struct node * get(struct node * root, struct node n);

void activate(struct node * root, struct node n);

void deactivate(struct node * root, struct node n);

void activateelev(struct elevator elev);

void deactivateelev(struct elevator elev);

void order_add_order(struct order ord);

void cleartable(int table[][N_PANELS]);


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
