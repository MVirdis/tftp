#include <pthread.h>

struct user {
	int id;
	struct sockaddr* addr;
	char* filename;
	pthread_cond_t acked;
	struct user* next;
};

typedef struct user* user_list_t;

void init_user_list(user_list_t* list);
int add(user_list_t* list, struct user* new_user);
void deallocate(struct user* user);
