#include <pthread.h>

struct transfer {
	int id;
	int chunks;
	int done;
	struct sockaddr* addr;
	char* filepath;
	pthread_cond_t acked;
	struct transfer* next;
};

typedef struct transfer* transfer_list_t;

void init_transfer_list(transfer_list_t* list);
int add(transfer_list_t* list, struct transfer* new_transfer);
void deallocate(struct transfer* transfer);
