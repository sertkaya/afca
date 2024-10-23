/*
 * AFCA - argumentation framework using closed sets
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "linked_list.h"

ListNode *create_list_node(void* c) {
	ListNode *n = calloc(sizeof(ListNode), 1);
	assert(n != NULL);
	n->c = c;
	n->next = NULL;
	return(n);
}


ListNode *free_list_node(ListNode *n) {
	// TODO: restructure code, improve
	// n->c should be freed before, not the best implementation.
	free(n);
	return(NULL);
}


ListNode *insert_list_node(void *c, ListNode *head) {
	ListNode *new_node = create_list_node(c);
	new_node->next = head;
	return(new_node);
	/*
	ListNode *next = head->next;
	head->next = create_list_node(c);
	head->next->next = next;
	return head->next;
	*/
}

size_t count_nodes(ListNode* node) {
	size_t n = 0;
	while (node) {
		++n;
		node = node->next;
	}
	return n;
}