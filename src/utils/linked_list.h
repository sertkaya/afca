/*
 * linked_list.h
 *
 *  Created on: 12.04.2023
 *      Author: bs
 */

#ifndef UTILS_LINKED_LIST_H_
#define UTILS_LINKED_LIST_H_

#include "../fca/concept.h"

struct list_node {
	Concept *c;
	struct list_node *next;
};

typedef struct list_node ListNode;

ListNode *create_node(Concept *c);

// Delete a node given with its predecessor from a list given with its head
ListNode *delete_node(ListNode *head, ListNode *previous, ListNode *node);

#endif /* UTILS_LINKED_LIST_H_ */