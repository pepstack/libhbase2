/**
 * wait_done.h
 */
#ifndef WAIT_DONE_H_INCLUDED
#define WAIT_DONE_H_INCLUDED

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>


typedef struct wait_done_t
{
    pthread_cond_t cond;
    pthread_mutex_t mutex;

    void *opaque;

    volatile int done;

    int result;

    ssize_t msgsize;
    char msg[0];
} wait_done_t;


static wait_done_t * wait_done_create (ssize_t msgsize, void *opaque)
{
    wait_done_t * p = (wait_done_t *) malloc(sizeof(*p) + msgsize);
    if (p) {
        p->opaque = opaque;
        p->done = 0;
        p->result = 0;
        p->msgsize = msgsize;

        pthread_cond_init(&p->cond, 0);
        pthread_mutex_init(&p->mutex, 0);
    }
    return p;
}


static void wait_done_discard (wait_done_t *p)
{
    if (p) {
        pthread_mutex_destroy(&p->mutex);
        pthread_cond_destroy(&p->cond);
        free(p);
    }
}


static void wait_done_set (wait_done_t *p, int result, char *msg)
{
    pthread_mutex_lock(&p->mutex);

    p->done = 1;
    p->result = result;

    if (msg && p->msgsize) {
        snprintf(p->msg, p->msgsize, "%s", msg);
    }

    pthread_cond_signal(&p->cond);
    pthread_mutex_unlock(&p->mutex);    
}


static void wait_done_until(wait_done_t *p)
{
    pthread_mutex_lock(&p->mutex);
    while (! p->done) {
        pthread_cond_wait(&p->cond, &p->mutex);
    }
    pthread_mutex_unlock(&p->mutex);
}


#if defined(__cplusplus)
}
#endif

#endif /* WAIT_DONE_H_INCLUDED */
