#include <pthread.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

struct msg {
	char *buf;
    uint32_t len;
    struct msg *m_next;
};


struct msg *workq;
pthread_cond_t qready = PTHREAD_COND_INITIALIZER;
pthread_mutex_t qlock = PTHREAD_MUTEX_INITIALIZER;

void process_msg()
{
    struct msg *mp;

    for(;;) {
        pthread_mutex_lock(&qlock);
        while (workq == NULL) {
            pthread_cond_wait(&qready, &qlock);
        }
        mp = workq;
        workq = mp->m_next;
        pthread_mutex_unlock(&qlock);
        printf("processing msg\n");
        printf("msg: %s\n", mp->buf);
    }
}

void enqueue_msg(struct msg *mp)
{
    pthread_mutex_lock(&qlock);
    mp->m_next = workq;
    workq = mp;
    pthread_mutex_unlock(&qlock);
    pthread_cond_signal(&qready);
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

int send_msg(char *buf, uint32_t len)
{
    enqueue_msg(make_msg(buf, len));

    return 0;
}

int recv_msg()
{
    process_msg();
}


void* start_func1(void * params)
{
    char *message = "hello,world";
    int i;


    for(i = 0; i < 10; i++) {
        printf("sending message: %s", message);
        send_msg(message, strlen(message) - i);
    }


	return NULL;
}

void* start_func2(void * params)
{
    char *message = "hello,world";

    recv_msg();

	return NULL;
}

int main()
{
	pthread_t thread1, thread2;
	void *retval;
	
	pthread_create(&thread1, NULL, start_func1, NULL);
	pthread_create(&thread2, NULL, start_func2, NULL);

	pthread_join(thread1, &retval);
	pthread_join(thread2, &retval);

	return 0;
}








