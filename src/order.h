#ifndef _ORDER_H
#define _ORDER_H

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

#endif
