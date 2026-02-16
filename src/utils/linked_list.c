/*
 * CLAS - Closure based Argumentation Solver
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
// Insert at head of the list
ListNode *insert_list_int(int n, ListNode *head) {
	ListElement *element = new_list_element_int(n);
	ListNode *new_node = new_list_node(element);
	new_node->next = head;
	return(new_node);
}

ListNode *insert_list_ptr(void *p, ListNode *head) {
	ListElement *element = new_list_element_ptr(p);
	ListNode *new_node = new_list_node(element);
	new_node->next = head;
	return(new_node);
}

ListNode *new_list_node(ListElement *e) {
	ListNode *node = calloc(1, sizeof(ListNode));
	assert(node != NULL);
	node->e = e;
	node->next = NULL;

	return(node);
}

ListElement *new_list_element_int(int n) {
	ListElement *element = calloc(1, sizeof(ListElement));
	assert(element != NULL);
	element->n = n;

	return(element);
}

inline ListElement *new_list_element_ptr(void *p) {
	ListElement *element = calloc(1, sizeof(ListElement));
	assert(element != NULL);
	element->p = p;

	return(element);
}

void print_linked_list(ListNode* head, void (*print_list_element)(ListElement *e, FILE *file, const char *end), FILE *out_file) {
	while (head) {
		print_list_element(head->e, out_file, "\n");
		head = head->next;
	}
}

int free_list(ListNode *head, void (*free_list_element)(ListElement *e)) {
	ListNode *previous = NULL;
	ListNode *current = head;
	while (current) {
		free_list_element(current->e);
		previous = current;
		current = current->next;
		free(previous);
	}
}