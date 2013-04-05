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

//Handle_button_lamps control all panel lights. 									
void order_handle_button_lamps();

// Function available for service engineers for testing.
void order_print(void);

/* Linked list for keeping track of all elevators online */
#define FLOORS 4

struct orderlist{
	int panel_cmd[FLOORS];
	int panel_up[FLOORS];
	int panel_down[FLOORS];
};
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
	struct orderlist current_orders;
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


void order_add_order(struct order ord);




#define PANEL_CMD 2
#define PANEL_UP 0
#define PANEL_DOWN 1

struct node * getelevnode(struct node n);

void addelev(struct elevator elev);

void addorder(struct node * elevnode, struct order ordr);


#endif
