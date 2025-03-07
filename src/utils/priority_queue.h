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

#ifndef UTILS_PRIORITY_QUEUE_H_
#define UTILS_PRIORITY_QUEUE_H_

#include <stdlib.h>
#include <assert.h>

union queue_element {
	int n;
	void *p;
};
typedef union queue_element QueueElement;

struct queue_node {
	QueueElement *e;
	struct queue_node *next;
	int priority;
};
typedef struct queue_node QueueNode;

QueueElement *new_queue_element_int(int n);

QueueElement *new_queue_element_ptr(void *p);

QueueNode *new_queue_node(QueueElement *e);

// Delete  node
void free_queue_element(QueueElement *e);

int free_queue(QueueNode *head, void (*free_queue_element)(QueueElement *e));

// Insert at head of the queue
QueueNode *enqueue_int(int n, QueueNode *head, int priority);

QueueNode *enqueue_ptr(void *p, QueueNode *head, int priority);

void *dequeue_ptr(QueueNode **head);

void print_linked_queue(QueueNode* head, void (*print_queue_element)(QueueElement *e, FILE *file, const char *end), FILE* out_file);

#endif /* UTILS_PRIORITY_QUEUE_H_ */
