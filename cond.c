#include <pthread.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define NUM_QUEUES 2
#define NUM_SENDER 2
#define NUM_RECVER 1


struct bc_msg {
    uint32_t event;                /* 下面的bc_event事件 */
    uint32_t session_id;           
    uint32_t len;
	char *buf;
    struct bc_msg *m_next;
};

void *del_msg(struct bc_msg *m);

struct bc_msg* module_msg_queues[NUM_QUEUES];
pthread_cond_t qready[NUM_QUEUES];
pthread_mutex_t qlock[NUM_QUEUES];

uint32_t init_queue_comm()
{
    uint32_t i;

    for (i = 0; i < NUM_QUEUES; i++) {
        pthread_mutex_init(qlock + i, NULL);
        pthread_cond_init(qready + i, NULL);
    }
}
        

void process_msg(char *buf, uint32_t *len, uint32_t module_id)
{
    struct bc_msg *mp;

//    fprintf(stderr,"waiting msg for module %d...\n", module_id);
    pthread_mutex_lock(qlock + module_id);
    while (module_msg_queues[module_id] == NULL) {
        pthread_cond_wait(qready + module_id, qlock + module_id);
    }
    mp = module_msg_queues[module_id];
    module_msg_queues[module_id] = mp->m_next;
    pthread_mutex_unlock(qlock + module_id);
    fprintf(stderr, "msg recv for module %d: %s, %d\n", module_id, mp->buf, mp->len);
    del_msg(mp);
}

void enqueue_msg(struct bc_msg *mp, uint32_t module_id)
{
    pthread_mutex_lock(qlock + module_id);
    mp->m_next = module_msg_queues[module_id];
    module_msg_queues[module_id] = mp;
    pthread_mutex_unlock(qlock + module_id);
    pthread_cond_signal(qready + module_id);
    fprintf(stderr,"message sent for module %d: %s, %d\n", module_id, mp->buf, mp->len);
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
int send_msg(struct bc_msg *msg, uint32_t module_id)
{
    enqueue_msg(msg, module_id);

    return 0;
}

int recv_msg(struct bc_msg **mp, uint32_t module_id, uint32_t *ev)
{
    pthread_mutex_lock(qlock + module_id);
    while (module_msg_queues[module_id] == NULL) {
        pthread_cond_wait(qready + module_id, qlock + module_id);
    }
    *mp = module_msg_queues[module_id];
    module_msg_queues[module_id] = (*mp)->m_next;
    (*mp)->m_next = NULL;
    pthread_mutex_unlock(qlock + module_id);
    *ev = (*mp)->event;
}


void* send_thread(void * params)
{
    char *message = "hello,world";
    uint32_t i;


    for(i = 0; i < 2; i++) {
        send_msg(make_msg(message, strlen(message)), i % NUM_QUEUES);
    }


	return NULL;
}

void* recv_thread(void * params)
{
    char *buf;
    uint32_t len;
    uint32_t i;
    struct bc_msg *m;
    uint32_t event;
    uint32_t module_id;

    for (i = 0; i < 9; i++) {
        module_id = i % NUM_QUEUES;
        recv_msg(&m, module_id, &event);
        fprintf(stderr, "msg recv for module %d: %s, %d, %d\n", module_id, m->buf, m->len, event);
    }

	return NULL;
}

int main()
{
	pthread_t sthreads[NUM_SENDER], rthreads[NUM_RECVER];
	void *retval;
    init_queue_comm();
    int i;
	
    for (i = 0; i < NUM_SENDER; i++) {
        pthread_create(sthreads+i, NULL, send_thread, NULL);
    }
    for (i = 0; i < NUM_RECVER; i++) {
        pthread_create(rthreads+i, NULL, recv_thread, NULL);
    }

    for (i = 0; i < NUM_SENDER; i++) {
        pthread_join(sthreads[i], &retval);
    }
    for (i = 0; i < NUM_RECVER; i++) {
        pthread_join(rthreads[i], &retval);
    }

	return 0;
}








