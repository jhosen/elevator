

int weightfunction(struct node* root, struct order new_order) {
	int n_elevs = count(root);
	int weight[n_elevs];
	int lowest_weight = 1000;
	int current = 0;
	struct node *iter;
	iter = root;
	int floor_counter;
	int ordered_floor = new_order->floor;
	int current_floor;
	while(iter!=0) {
		//Add weight if elevator has order to floors (different weighting for different floors)
		for(floor_counter=0;floor_counter<N_FLOORS; floor_counter++) {
			if(iter->elevinfo->current_orders->panel_cmd[floor_counter]) {
				if(floor_counter == ordered_floor) {
					weight[current]-=10;
				}
				else {
					weight[current]+=3;
				}
			}
			if(floor_counter<N_FLOORS-1 && iter->elevinfo->current_orders->panel_up[floor_counter]) {
				if(floor_counter == ordered_floor) {
					weight[current]-=5;
				}
				else {
					weight[current]+=3;
				}
			}
			if(floor_counter>0 && iter->elevinfo->current_order->panel_down[floor_counter]) {
				if(floor_counter == ordered_floor) {
					weight[current]-=5;
				}
				else {
					weight[current]+=3;
				}
			}
		}
		//Add weight based on the number of floors the elevator will pass before reaching the ordered floor
		current_floor = iter->elevinfo->current_state->last_floor;
		current_dir = iter->elevinfo->current_state->last_dir;
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
		current++;
		iter = iter->next;
	}
	int i=0;
	int chosen_elevator;
	for(i=0; i<n_elevs; i++) {
		if(weight[i]<lowest_weight) {
			lowest_weight = weight[i];
			chosen_elevator = i;
		}
	}
}

int find(struct node * root, struct node n) {
	struct node *iter;
	iter = root;
	while(iter!=0){
		if(iter->id == n.id){
			// do something
			return 1;
		}
		iter = iter->next;
	}
	return 0
}
