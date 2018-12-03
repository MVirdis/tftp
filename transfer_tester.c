#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <unistd.h>

#include "transfer.h"

transfer_list_t list;

void* test_threading(void* args) {

	struct transfer* new_transfer = (struct transfer*)args;

	pthread_mutex_lock(&new_transfer->mutex);
	pthread_cond_wait(&new_transfer->acked, &new_transfer->mutex);
	pthread_mutex_unlock(&new_transfer->mutex);

	printf("transfer_threading: OK\n");

	pthread_exit(NULL);
}

int main() {
	//  Testo la init
	init_transfer_list(&list);
	if (list == NULL)
		printf("init_transfer_list: OK\n");
	else
		printf("init_transfer_list: X\n");

	// Testo la create_transfer
	struct sockaddr_in addr;
	pthread_t t;
	char paddr[INET_ADDRSTRLEN];
	memset(&addr, 0, sizeof addr);
	addr.sin_family = AF_INET;
	inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);
	addr.sin_port = htons(4242);
	struct transfer* new_transfer = create_transfer(0, (struct sockaddr*)&addr, "files/test.txt", 0);
	print_transfer_list(list);
	pthread_create(&t, NULL, test_threading, (void*)new_transfer);
	sleep(1);
	pthread_mutex_lock(&new_transfer->mutex);
	pthread_cond_signal(&new_transfer->acked);
	pthread_mutex_unlock(&new_transfer->mutex);
	sleep(1);
	if(new_transfer->filepath == NULL)
		printf("create_transfer: X\n");
	if(new_transfer->next != NULL)
		printf("create_transfer: X\n");

	// Testo la add
	add(&list, new_transfer);
	

	return 0;
}
