/*
 * linked_list.c
 *
 *  Created on: 12.04.2023
 *      Author: bs
 */


#include <stdio.h>

#include "../fca/concept.h"
#include "linked_list.h"

ListNode *create_node(Concept* c) {
	ListNode *n = calloc(sizeof(ListNode), 1);
	assert(n != NULL);
	n->c = c;
	n->next = NULL;
	return(n);
}

ListNode *free_node(ListNode *n) {
	// TODO: restructure code, improve
	// n->c should be freed before, not the best implementation.
	free(n);
	return(NULL);
}
