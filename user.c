#include "user.h"

void init_user_list(user_list_t* list) {
	*list = NULL;
}

int add_user(user_list_t* list, struct user* new_user) {
	struct user* it;
	if (list == NULL) {
		*list = new_user;
		new_user->next = NULL;
		return 0;
	}
	for(it=*list; it->next; it=it->next); // Scorro fino all'ultimo
	it->next = new_user;
	return 0;
}
