#ifndef _BC_COMM_
#define _BC_COMM_

enum module_names {
    MODULE_1,
    MODULE_2,
    MODULE_3,
    MODULE_4,
    MODULE_5,
    N_MODULE
};

struct bc_msg {
    uint32_t event;                /* 下面的bc_event事件 */
    uint32_t session_id;           
    uint32_t len;
	char *buf;
    struct bc_msg *m_next;
};

struct bc_msg_head {
    pthread_cond_t qready;
    pthread_mutex_t qlock;
    struct bc_msg *next;
};

typedef struct bc_msg bc_msg_t;
typedef struct bc_msg_head bc_msg_head_t;


int init_queue_comm(uint32_t qsize);
struct bc_msg *make_msg(char *buf, uint32_t len);
void *del_msg(struct bc_msg *m);
int send_msg(struct bc_msg *mp, uint32_t module_id);
int recv_msg(struct bc_msg **mp, uint32_t module_id, uint32_t *ev);

#endif /* _BC_COMM_ */
