#include <stdio.h>

#include "list.h"

/* list helper functions */
int list_size(thread_info_list *list)
{
	int cnt = 0;

	if (!list) return -1;

	pthread_mutex_lock(&list->lock);
	list_elem *le = list->head;
	while (le) {
		cnt++;
		le = le->next;
	}
	pthread_mutex_unlock(&list->lock);

	return cnt;
}

int list_insert_head(thread_info_list *list, list_elem *new)
{
	if (!list || !new) return -1;

	pthread_mutex_lock(&list->lock);
	new->next = list->head;
	new->prev = 0;
	if (new->next) {
		new->next->prev = new;
	}
	list->head = new;
	if (list->tail == 0) {
		list->tail = new;
	}
	pthread_mutex_unlock(&list->lock);
	return 0;
}

int list_insert_tail(thread_info_list *list, list_elem *new)
{
	if (!list || !new) return -1;

	pthread_mutex_lock(&list->lock);
	new->prev = list->tail;
	new->next = 0;

	if (new->prev) {
		new->prev->next = new;
	}
	list->tail = new;
	if (list->head == 0) {
		list->head = new;
	}
	pthread_mutex_unlock(&list->lock);

	return 0;
}

int list_remove(thread_info_list *list, list_elem *old)
{
	if (!old || !list) return -1;

	pthread_mutex_lock(&list->lock);
	if (old->next) {
		old->next->prev = old->prev;
	}
	if (old->prev) {
		old->prev->next = old->next;
	}
	if (list->tail == old) {
		list->tail = old->prev;
	}
	if (list->head == old) {
		list->head = old->next;
	}

	old->next = old->prev = 0;
	pthread_mutex_unlock(&list->lock);

	return 0;
}

void print_list(thread_info_list *list)
{
	pthread_mutex_lock(&list->lock);
	list_elem *le = list->head;
	while (le) {
		printf("0x%X,", (unsigned int)le->info);
		le = le->next;
	}
	pthread_mutex_unlock(&list->lock);
	printf("\n");
}
