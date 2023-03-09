#ifndef __LIST_H_
#define __LIST_H_

#include <pthread.h>

typedef struct list_elem {
	struct list_elem	*prev;
	struct list_elem	*next;
	void			*info;
} list_elem;

typedef struct thread_info_list {
	list_elem	*head;
	list_elem	*tail;
	pthread_mutex_t	lock;
} thread_info_list;
/* Compute the size of a list (O(n) time). */
int list_size(thread_info_list *list);
/* Insert an element at the head of a list.*/
int list_insert_head(thread_info_list *list, list_elem *new);
/* Insert an element at the tail of a list.*/
int list_insert_tail(thread_info_list *list, list_elem *new);
/* Removes an element from a list (assuming it's in the list). */
int list_remove(thread_info_list *list, list_elem *new);
void print_list(thread_info_list *list);
#endif /* __LIST_H_ */

