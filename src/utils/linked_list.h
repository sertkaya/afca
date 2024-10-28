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

#ifndef UTILS_LINKED_LIST_H_
#define UTILS_LINKED_LIST_H_

struct list_node {
	void *c;
	struct list_node *next;
};

typedef struct list_node ListNode;

ListNode *create_list_node(void *c);

// Delete  node
ListNode *free_list_node(ListNode *n);

void free_list(ListNode *head, void (*free_list_element)(void *e));

// Insert at head of the list
ListNode *insert_list_node(void *c, ListNode *head);

size_t count_nodes(ListNode* node);

void print_list(ListNode* head, void (*print_list_element)(void *e, FILE *file, const char *end), FILE* out_file);

#endif /* UTILS_LINKED_LIST_H_ */
