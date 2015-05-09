#include <pthread.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define NUM_QUEUES 2


struct msg {
	char *buf;
    uint32_t len;
    struct msg *m_next;
};

void *del_msg(struct msg *m);

struct msg* module_msg_queues[NUM_QUEUES];
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
    struct msg *mp;

    fprintf(stderr,"waiting msg for module %d...\n", module_id);
    pthread_mutex_lock(qlock + module_id);
    while (module_msg_queues[module_id] == NULL) {
        pthread_cond_wait(qready + module_id, qlock + module_id);
    }
    mp = module_msg_queues[module_id];
    module_msg_queues[module_id] = mp->m_next;
    pthread_mutex_unlock(qlock + module_id);
    printf("msg recv for module %d: %s\n", module_id, mp->buf);
    del_msg(mp);
}

void enqueue_msg(struct msg *mp, uint32_t module_id)
{
    pthread_mutex_lock(qlock + module_id);
    mp->m_next = module_msg_queues[module_id];
    module_msg_queues[module_id] = mp;
    pthread_mutex_unlock(qlock + module_id);
    pthread_cond_signal(qready + module_id);
    printf("message sent for module %d\n", module_id);
}

struct msg *make_msg(char *buf, uint32_t len)
{
    struct msg *m;

    m = (struct msg *)malloc(sizeof(struct msg));
    m->buf = (char *)malloc(len);
    strncpy(m->buf, buf, len);
    m->len = len;

    return m;
}

void *del_msg(struct msg *m)
{
    free(m->buf);
    free(m);
}

/* send message to a module */
int send_msg(char *buf, uint32_t len, uint32_t module_id)
{
    enqueue_msg(make_msg(buf, len), module_id);

    return 0;
}

int recv_msg(char *buf, uint32_t *len, uint32_t module_id)
{
    process_msg(buf, len, module_id);
}


void* start_func1(void * params)
{
    char *message = "hello,world";
    uint32_t i;


    for(i = 0; i < 2; i++) {
        fprintf(stderr, "sending message: %s\n", message);
        send_msg(message, strlen(message) - i, i % NUM_QUEUES);
    }


	return NULL;
}

void* start_func2(void * params)
{
    char *message = "hello,world";
    char *buf;
    uint32_t len;
    uint32_t i;

    for (i = 0; i < 2; i++) {
        recv_msg(buf, &len, i % NUM_QUEUES);
    }

	return NULL;
}

int main()
{
	pthread_t thread1, thread2;
	void *retval;
    init_queue_comm();
	
	pthread_create(&thread1, NULL, start_func1, NULL);
    sleep(1);
	pthread_create(&thread2, NULL, start_func2, NULL);

	pthread_join(thread2, &retval);
	pthread_join(thread1, &retval);

	return 0;
}








