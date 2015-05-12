
#include <pthread.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>

#include "communication.h"

#define NUM_SENDER N_MODULE
#define N_MSG_PER_SENDER 1000

struct vars {
    uint32_t id;
};

static uint32_t scount;
static uint32_t rcount;
pthread_mutex_t slock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t rlock = PTHREAD_MUTEX_INITIALIZER;



void* send_thread(void * params)
{
    char *message;
    char *mesg = "hello,world";
    uint32_t i;
    uint32_t to_module;
    int ret;

    srand(time(NULL));
    for(i = 0; i < N_MSG_PER_SENDER; i++) {
        message = (char *)malloc(sizeof(char) * (strlen(mesg)+1));
        if (message == NULL) {
            fprintf(stderr, "malloc error\n");
            exit(1);
        }
        strncpy(message, mesg, strlen(mesg));

        to_module = rand() % N_MODULE;
        ret = send_msg(to_module, 0, 0, message, strlen(mesg)+1);
        assert(ret == 0);
        pthread_mutex_lock(&slock);
        scount++;
        pthread_mutex_unlock(&slock);
        fprintf(stderr,"sent %d\n", scount);
    }

	return NULL;
}

void* recv_thread(void * params)
{
    char *buf;
    uint32_t len;
    uint32_t i;
    struct bc_msg *m;
    uint32_t module_id;
    int ret;

    module_id = ((struct vars *)params)->id;

    while (1) {
        ret = recv_msg(module_id, &m);
        assert(ret == 0);
        del_msg(m);
        pthread_mutex_lock(&rlock);
        rcount++;
        fprintf(stderr, "recv %d\n", rcount);
        pthread_mutex_unlock(&rlock);
    }

	return NULL;
}

int main()
{
	pthread_t sthreads[NUM_SENDER], rthreads[N_MODULE];
	void *retval;
    uint32_t i;
    struct vars v[N_MODULE];

    init_queue_comm();
    scount = 0;
    rcount = 0;
	

    for (i = 0; i < NUM_SENDER; i++) {
        pthread_create(sthreads+i, NULL, send_thread, NULL);
    }
    for (i = 0; i < N_MODULE; i++) {
        v[i].id = i;
        pthread_create(rthreads+i, NULL, recv_thread, (void *)(v+i));
    }


    for (i = 0; i < NUM_SENDER; i++) {
        pthread_join(sthreads[i], &retval);
    }
    for (i = 0; i < N_MODULE; i++) {
        pthread_join(rthreads[i], &retval);
    }

	return 0;
}


