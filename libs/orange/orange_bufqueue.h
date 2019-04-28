#ifndef  ORANGE_BUFQUEUE_H
#define  ORANGE_BUFQUEUE_H
#include "orange.h"

typedef struct orange_bufqueue {
    uint32_t    head;
    uint32_t    tail;
    int         size;
    int         mask;
    int         count;
    void        *queue[0];
} orange_bufqueue_t;

static __inline int orange_bufqueue_full(struct orange_bufqueue *bq)
{
	return (bq->size == bq->count);
}

static __inline int orange_bufqueue_empty(struct orange_bufqueue *bq)
{
	return (bq->count == 0);
}

static __inline int orange_bufqueue_count(struct orange_bufqueue *bq)
{
    return bq->count;
}


static __inline int orange_bufqueue_enqueue(struct orange_bufqueue *bq, void *buf)
{
	uint32_t head, next;

	if (bq->count == bq->size) {
		return (ENOBUFS);
	}

	head = bq->head;

	next = (head + 1) & bq->mask;

    bq->head = next;
	bq->queue[head] = buf;
	bq->count++;

    return (0);
}

static __inline void * orange_bufqueue_dequeue(struct orange_bufqueue *bq)
{
	uint32_t next;
	uint32_t tail;
	void *buf;

	if (bq == NULL || bq->count == 0) {
		return NULL;
    }

	tail = bq->tail;

    next = (tail + 1) & bq->mask;
    bq->tail = next;
    bq->count--;

	buf = bq->queue[tail];

	return (buf);
}

/* buf ring count must be size power of 2 */
extern struct orange_bufqueue *orange_bufqueue_alloc(int count);
extern void orange_bufqueue_free(struct orange_bufqueue *bq);

#endif

