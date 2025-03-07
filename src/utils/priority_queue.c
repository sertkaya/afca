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

#include "priority_queue.h"

#include <time.h>

QueueNode *enqueue_int(int n, QueueNode *head, int priority) {
	QueueNode *previous = NULL;
	QueueNode *current = head;

	while (current != NULL && priority > current->priority) {
		previous = current;
		current = current->next;
	}

	QueueElement *element = new_queue_element_int(n);
	QueueNode *new_node = new_queue_node(element);

	new_node->priority = priority;
	new_node->next = current;

	if (previous == NULL) {
		return(new_node);
	}

	previous->next = new_node;

	return(head);
}

// insert sorted according to decreasing priorities
QueueNode *enqueue_ptr(void *p, QueueNode *head, int priority) {
	QueueNode *previous = NULL;
	QueueNode *current = head;

	// search the correct place to insert
	while (current != NULL && priority > current->priority) {
		previous = current;
		current = current->next;
	}

	QueueElement *element = new_queue_element_ptr(p);
	QueueNode *new_node = new_queue_node(element);

	new_node->priority = priority;
	new_node->next = current;

	if (previous == NULL) {
		return(new_node);
	}

	previous->next = new_node;

	return(head);
}

void *dequeue_ptr(QueueNode **head) {
	// check if the queue is empty
	if ((*head) == NULL)
		return(NULL);
	// get the pointer stored in the first queue element
	void *ptr = (*head)->e->p;
	// store the head in tmp
	QueueNode *tmp = *head;
	// new head is the next of head
	*head = (*head)->next;
	// free the element at the old head
	free(tmp->e);
	free(tmp);
	// return the stored pointer
	return(ptr);
}

QueueNode *new_queue_node(QueueElement *e) {
	QueueNode *node = calloc(1, sizeof(QueueNode));
	assert(node != NULL);
	node->e = e;
	node->next = NULL;

	return(node);
}

QueueElement *new_queue_element_int(int n) {
	QueueElement *element = calloc(1, sizeof(QueueElement));
	assert(element != NULL);
	element->n = n;

	return(element);
}

QueueElement *new_queue_element_ptr(void *p) {
	QueueElement *element = calloc(1, sizeof(QueueElement));
	assert(element != NULL);
	element->p = p;

	return(element);
}


void print_linked_queue(QueueNode* head, void (*print_queue_element)(QueueElement *e, FILE *file, const char *end), FILE *out_file) {
	while (head) {
		print_queue_element(head->e, out_file, "\n");
		head = head->next;
	}
}

int free_queue(QueueNode *head, void (*free_queue_element)(QueueElement *e)) {
	QueueNode *previous = NULL;
	QueueNode *current = head;
	while (current) {
		free_queue_element(current->e);
		previous = current;
		current = current->next;
		free(previous);
	}
}