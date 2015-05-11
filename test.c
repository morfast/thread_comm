
#include <pthread.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>

#include "communication.h"

#define NUM_SENDER N_MODULE

struct vars {
    uint32_t id;
};


void* send_thread(void * params)
{
    char *message = "hello,world";
    uint32_t i;
    uint32_t to_module;
    int ret;

    srand(time(NULL));
    for(i = 0; i < 900; i++) {
        to_module = rand() % N_MODULE;
        ret = send_msg(make_msg(message, strlen(message)+1), to_module);
        assert(ret == 0);
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
    int ret;

    module_id = ((struct vars *)params)->id;

    while (1) {
        ret = recv_msg(&m, module_id, &event);
        assert(ret == 0);
        fprintf(stderr, "recv %d: %s, %d, %d\n", module_id, m->buf, m->len, event);
        del_msg(m);
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


