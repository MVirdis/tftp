#include "transfer.h"

#include <pthread.h>
#include <stdlib.h>

void init_transfer_list(transfer_list_t* list) {
	*list = NULL;
}

int add(transfer_list_t* list, struct transfer* new_transfer) {
	struct transfer* it;
	if (list == NULL) return -1;
	if (*list == NULL) {
		*list = new_transfer;
		new_transfer->next = NULL;
		return 0;
	}
	for(it=*list; it->next; it=it->next); // Scorro fino all'ultimo
	it->next = new_transfer;
	return 0;
}

void deallocate(struct transfer* transfer) {
	if (!transfer) return;
	if (transfer->addr)
		free(transfer->addr);
	if (transfer->filename)
		free(transfer->filename);
	free(transfer);
}
