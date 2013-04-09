//
//  buffer_elev.h
//  
//
//  Created by Jesper Hosen on 04.04.13.
//
//

#ifndef _buffer_elev_h
#define _buffer_elev_h



#define OVERWRITE_BUFFER_ON_OVERFLOW	1 //When overflow on internal buffer/queue occurs this determines whether you should overwrite existing data or not.



/*! \brief Type-defining a CircularBuffer type for internal buffering
 * when receiving and transmitting data by network. CircularBuffer is FIFO.
 */
typedef struct {
	int				size;   /* maximum number of elements           */
	int				start;  /* index of oldest element              */
	int 			count;  /* index at which to write new element	*/
	struct msg	   *elems;  /* vector of elements                   */
} CircularBuffer;




/*! \brief Initiates the two local buffers/queues from main application.
 */
void init_main_cb(void);

/*! \brief Sets start and counter attribute of cb to zero,
 * and size to size.
 * This function is run by init_main_cb().
 */
void cbInit(CircularBuffer *cb, int size);

/*! \brief Evaluates to true if buffer is full.
 * Return false if there is still more space.
 */
int cbIsFull(CircularBuffer *cb);

/*! \brief Return true if buffer is empty.
 * \param cb is either cb_load or cb_store
 */
int cbIsEmpty(CircularBuffer *cb);

/*! \brief loads buffer with a msg packet type.
 * \param cb is the buffer/queue we want to write to.
 * \param elem is the element we want to put on the queue.
 */
void cbWrite(CircularBuffer *cb, struct msg *elem);

/*! \brief Copy a element from cb and paste it in elem.
 * This function also deletes the element it reads.
 * Function is implemented FIFO.
 * \param cb is the buffer/queue we want to read from.
 * \param elem is the place we want to store element from cb.
 *
 */
void cbRead(CircularBuffer *cb, struct msg *elem);

#endif
