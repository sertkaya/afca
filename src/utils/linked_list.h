/*
 * linked_list.h
 *
 *  Created on: 12.04.2023
 *      Author: bs
 */

#ifndef UTILS_LINKED_LIST_H_
#define UTILS_LINKED_LIST_H_

struct list_node {
	void *c;
	struct list_node *next;
};

typedef struct list_node ListNode;

ListNode *create_node(void *c);

// Delete  node
ListNode *free_node(ListNode *n);

ListNode *insert_node(void *c, ListNode *prev);

size_t count_nodes(ListNode* node);

#endif /* UTILS_LINKED_LIST_H_ */
