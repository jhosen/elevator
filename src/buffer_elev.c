//
//  buffer_elev.c
//  
//
//  Created by Jesper Hosen on 04.04.13.
//
//

#include <stdio.h>
#include "buffer_elev.h"
#include "network.h"
#include "communication.h"
//static CircularBuffer buffer;


void cbInit(CircularBuffer *cb, int size) {
	cb->size  = size;
	cb->start = 0;
	cb->count = 0;
	cb->elems = (struct msg *)calloc(cb->size, sizeof(struct msg));
}

int cbIsFull(CircularBuffer *cb) {
	return cb->count == cb->size;
}

int cbIsEmpty(CircularBuffer *cb) {
	return cb->count == 0;
}

void cbWrite(CircularBuffer *cb, struct msg *elem) {
	int end = (cb->start + cb->count) % cb->size;
	cb->elems[end] = *elem;
	if (cb->count == cb->size){
		if(OVERWRITE_BUFFER_ON_OVERFLOW)
			cb->start = (cb->start + 1) % cb->size;	  //: Buffer is full, overwrite
		else
			NULL; //: Buffer is full, discard data
	}
	else
        ++ cb->count;
}

void cbRead(CircularBuffer *cb, struct msg *elem) {
	*elem = cb->elems[cb->start];
	cb->start = (cb->start + 1) % cb->size;
	-- cb->count;
}

