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

union list_element {
	int n;
	void *p;
};
typedef union list_element ListElement;

struct list_node {
	ListElement *e;
	struct list_node *next;
};
typedef struct list_node ListNode;

ListElement *new_list_element_int(int n);

inline ListElement *new_list_element_ptr(void *p) {
	ListElement *element = calloc(1, sizeof(ListElement));
	assert(element != NULL);
	element->p = p;

	return(element);
}

ListNode *new_list_node(ListElement *e);

// Delete  node
void free_list_element(ListElement *e);

int free_list(ListNode *head, void (*free_list_element)(ListElement *e));

// Insert at head of the list
ListNode *insert_list_int(int n, ListNode *head);

inline ListNode *insert_list_ptr(void *p, ListNode *head) {
	ListElement *element = new_list_element_ptr(p);
	ListNode *new_node = new_list_node(element);
	new_node->next = head;
	return(new_node);
}

void print_linked_list(ListNode* head, void (*print_list_element)(ListElement *e, FILE *file, const char *end), FILE* out_file);

#endif /* UTILS_LINKED_LIST_H_ */
