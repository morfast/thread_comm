#include <pthread.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "communication.h"

static struct bc_msg* module_msg_queues[NUM_QUEUES];
static pthread_cond_t qready[NUM_QUEUES];
static pthread_mutex_t qlock[NUM_QUEUES];

int init_queue_comm(uint32_t qsize)
{
    uint32_t i;

    for (i = 0; i < qsize; i++) {
        pthread_mutex_init(qlock + i, NULL);
        pthread_cond_init(qready + i, NULL);
    }

    return 0;
}
        
struct bc_msg *make_msg(char *buf, uint32_t len)
{
    struct bc_msg *m;

    m = (struct bc_msg *)malloc(sizeof(struct bc_msg));
    m->buf = (char *)malloc(len);
    strncpy(m->buf, buf, len);
    m->len = len;
    m->event = 0;
    m->session_id = 0;
    m->m_next = NULL;

    return m;
}

void *del_msg(struct bc_msg *m)
{
    free(m->buf);
    free(m);
}

/* send message to a module */
int send_msg(struct bc_msg *mp, uint32_t module_id)
{
    int ret;

    if ((ret = pthread_mutex_lock(qlock + module_id)) != 0)
        return 1;
    mp->m_next = module_msg_queues[module_id];
    module_msg_queues[module_id] = mp;
    if ((ret = pthread_mutex_unlock(qlock + module_id)) != 0)
        return 1;
    if ((ret = pthread_cond_signal(qready + module_id)) != 0)
        return 1;
    fprintf(stderr,"sent %d: %s, %d\n", module_id, mp->buf, mp->len);

    return 0;
}

int recv_msg(struct bc_msg **mp, uint32_t module_id, uint32_t *ev)
{
    int ret;

    if ((ret = pthread_mutex_lock(qlock + module_id)) != 0)
        return 1;
    while (module_msg_queues[module_id] == NULL) {
        pthread_cond_wait(qready + module_id, qlock + module_id);
    }
    *mp = module_msg_queues[module_id];
    module_msg_queues[module_id] = (*mp)->m_next;
    (*mp)->m_next = NULL;
    if ((ret = pthread_mutex_unlock(qlock + module_id)) != 0)
        return 1;
    *ev = (*mp)->event;

    return 0;
}


