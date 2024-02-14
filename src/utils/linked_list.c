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

ListNode *create_node(void* c) {
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


ListNode *insert_node(void *c, ListNode *prev) {
	ListNode *next = prev->next;
	prev->next = create_node(c);
	prev->next->next = next;
	return prev->next;
}
