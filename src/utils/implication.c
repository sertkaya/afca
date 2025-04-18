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

// requires: lhs is sorted
Implication *new_implication(ARG_TYPE *lhs, SIZE_TYPE lhs_size, ARG_TYPE *rhs, SIZE_TYPE rhs_size) {
	Implication *imp = calloc(1, sizeof(Implication));
	assert(imp != NULL);

	// list_copy(lhs, imp->lhs);
	imp->lhs = list_create();
	for (SIZE_TYPE i = 0; i < lhs_size; ++i)
		list_add(lhs[i], imp->lhs);
	// list_sort(imp->lhs);

	imp->rhs = list_create();
	for (SIZE_TYPE i = 0; i < rhs_size; ++i)
		list_add(rhs[i], imp->rhs);

	return (imp);
}

void print_implication(Implication *imp) {
	print_list(stdout, imp->lhs, "-> ");
	print_list(stdout, imp->rhs, "\n");
}

ListNode *create_implications_from_af(AF *af, AF *af_t) {
	ListNode *imps = NULL;
	int count = 0;
	for (SIZE_TYPE i = 0; i < af->size; ++i) {
		ARG_TYPE *tmp = calloc(af->list_sizes[i], sizeof(ARG_TYPE));
		assert(tmp != NULL);
		SIZE_TYPE tmp_count = 0;
		for (SIZE_TYPE j = 0; j < af->list_sizes[i]; ++j) {
			if (!check_arg_attacks_arg_sorted(af, af->lists[i][j], i)) {
				tmp[tmp_count++] = af->lists[i][j];
			}
		}

		Implication *imp = new_implication(af_t->lists[i], af_t->list_sizes[i], tmp, tmp_count);
		// free(conflict_free_victims);
		// printf("%d:", imp->lhs->size);
		// print_implication(imp);
		imps = insert_list_ptr(imp, imps);
		++count;
	}
	printf("%d implications\n", count);
	return (imps);
}

void implication_closure(ListNode *imps, ArrayList *s, ArrayList *closure) {
	bool update = false;
	do {
		update = false;
		ListNode *current = imps;
		while (current) {
			Implication *imp = current->e->p;
			// if (check_subset_sorted(((Implication*) current->e->p)->lhs, closure)) {
			if (check_subset_sorted(imp->lhs, closure)) {
				// TODO: Optimize!
				for (SIZE_TYPE i = 0; i < imp->rhs->size; ++i) {
					list_add(imp->rhs->elements[i], closure);
					update = true;
				}
				list_sort(closure);
			}
			current = current->next;
		}
	} while (update);
}
/*
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
*/