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

#include "implication.h"
#include "array_list.h"
#include "linked_list.h"
#include "stack.h"
#include "../af/af.h"

Implication *new_implication(ARG_TYPE *lhs, SIZE_TYPE lhs_size, ARG_TYPE rhs) {
	Implication *imp = calloc(1, sizeof(Implication));
	assert(imp != NULL);

	// list_copy(lhs, imp->lhs);
	imp->lhs = list_create();
	for (SIZE_TYPE i = 0; i < lhs_size; ++i)
		list_add(lhs[i], imp->lhs);
	// list_sort(imp->lhs);
	imp->rhs = rhs;

	return (imp);
}

void print_implication(Implication *imp) {
	print_list(stdout, imp->lhs, "-> ");
	printf("%d\n", imp->rhs);
}

ListNode *create_implications(AF *af, AF *af_t) {
	ListNode *imps = NULL;
	int count = 0;
	for (SIZE_TYPE attacker = 0; attacker < af->size; ++attacker) {
		for (SIZE_TYPE j = 0; j < af->list_sizes[attacker]; ++j) {
			Implication *imp = new_implication(af_t->lists[attacker], af_t->list_sizes[attacker],
			                                   af->lists[attacker][j]);
			// printf("%d:", imp->lhs->size);
			// print_implication(imp);
			imps = insert_list_ptr(imp, imps);
			++count;
		}
	}
	printf("%d implications\n", count);
	return (imps);
}

void close(ARG_TYPE x, ListNode *implications, bool *closure) {
	Stack update;
	init_stack(&update);
	push(&update, new_stack_element_int(x));

	ARG_TYPE a = -1;
	while ((a = pop_int(&update)) != -1) {
		ListNode *current = implications;
		while (current) {
			Implication *imp = current->e->p;
			if ((!closure[imp->rhs]) && ((imp->lhs->size == 0) || (
				                                imp->lhs->size == 1 && imp->lhs->elements[0] == a))) {
				closure[imp->rhs] = true;
				push(&update, new_stack_element_int(imp->rhs));
			}
			current = current->next;
		}
	}
}
