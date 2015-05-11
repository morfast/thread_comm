#include <pthread.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "communication.h"

static struct bc_msg_head module_msg_queues[N_MODULE];

int init_queue_comm()
{
    uint32_t i;
    struct bc_msg_head *head;

    for (i = 0; i < N_MODULE; i++) {
        head = module_msg_queues+i;
        pthread_mutex_init(&(head->qlock), NULL);
        pthread_cond_init(&(head->qready), NULL);
        head->next = NULL;
        head->end = NULL;
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
    struct bc_msg *msg;
    struct bc_msg_head *head;
    pthread_cond_t *qready;
    pthread_mutex_t *qlock;

    if (mp == NULL) {
        fprintf(stderr, "msg NULL\n");
        return 1;
    }
    if (module_id < 0 || module_id >= N_MODULE) {
        fprintf(stderr, "module_id error in send_msg\n");
        return 1;
    }

    head = module_msg_queues+module_id;
    qready = &(head->qready);
    qlock = &(head->qlock);

    if ((ret = pthread_mutex_lock(qlock)) != 0)
        return 1;
    msg = head->next;

    /* add mp to the end of queue */
    if (msg == NULL) {
        head->next = mp;
        head->end = mp;
    } else {
        head->end->m_next = mp;
        head->end = mp;
    }

    if ((ret = pthread_mutex_unlock(qlock)) != 0)
        return 1;
    if (msg == NULL) {
        if ((ret = pthread_cond_signal(qready)) != 0)
            return 1;
    }

    fprintf(stderr,"sent %d: %s, %d\n", module_id, mp->buf, mp->len);

    return 0;
}

int recv_msg(struct bc_msg **mp, uint32_t module_id, uint32_t *ev)
{
    int ret;
    struct bc_msg *msg;
    struct bc_msg_head *head;
    pthread_cond_t *qready;
    pthread_mutex_t *qlock;

    if (module_id < 0 || module_id >= N_MODULE) {
        fprintf(stderr, "module_id error in send_msg\n");
        return 1;
    }

    head = module_msg_queues+module_id;
    qready = &(head->qready);
    qlock = &(head->qlock);

    if ((ret = pthread_mutex_lock(qlock)) != 0)
        return 1;
    while (head->next == NULL) {
        printf("waiting in %d\n", module_id);
        pthread_cond_wait(qready, qlock);
    }
    /* get the first msg in the queue */
    msg = head->next;
    head->next = msg->m_next;
    if (head->next == NULL) {
        head->end = NULL;
    }
    (*mp) = msg;
    (*mp)->m_next = NULL;
    if ((ret = pthread_mutex_unlock(qlock)) != 0)
        return 1;
    *ev = (*mp)->event;

    return 0;
}


