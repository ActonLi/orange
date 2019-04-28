#include "orange_bufqueue.h"

struct orange_bufqueue *orange_bufqueue_alloc(int count)
{
	struct orange_bufqueue *bq;

    /* buf ring must be size power of 2 */
	assert(powerof2(count));
	
	bq = orange_zalloc(sizeof(struct orange_bufqueue) + count*sizeof(caddr_t));
	if (bq == NULL)
		return (NULL);

	bq->size = count;
    bq->mask = count - 1;
    bq->count = 0;
	bq->head = 0;
	bq->tail = 0;
		
	return (bq);
}

void orange_bufqueue_free(struct orange_bufqueue *br)
{
	orange_free(br);
}

