typedef user_list_t struct user*;

struct user {
	struct sockaddr* addr;
	pthread_cond_t acked;
	struct user* next;
};

void init_user_list(user_list_t* list);
int add(user_list_t* list, struct user* new_user);
