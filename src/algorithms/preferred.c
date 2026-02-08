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


#include <string.h>
#include <stdio.h>
#include <time.h>

#include "../utils/stack.h"
#include "../utils/timer.h"

#include "../af/af.h"
#include "preferred.h"

#include "node.h"
#include "closure.h"
#include "complete.h"

// A preferred extension is a maximal admissible set. Every preferred extension is pseudo-complete.
// A set is pseudo-complete if it is semi-complete and quasi-complete.
// A set is semi-complete if it is conflict-free and contains every argument that it defends.
// A set S is quasi-complete if it is conflict-free and contains every argument that is attacked
// by only attackers of S.
// Pseudo-complete sets form a closure system.

extern int closure_count;


bool* se_pr(AF* af) {
	struct timeval start_time, stop_time;

	// Array of AFs. The original AF and the residiual AFs generated in the loop are stored here
	// for later mapping the indices to the original indices.
	AF **afs = calloc(1, sizeof(AF*));
	assert(afs != NULL);
	SIZE_TYPE af_count = 1;
	afs[af_count - 1] = af;

	// the result
	bool *pe = calloc(af->size, sizeof(bool));
	assert(pe != NULL);

	while (afs[af_count - 1]->size > 0) {
		// printf("----af----:%d\n", afs[af_count]->size);
		// print_argumentation_framework(afs[af_count]);

		AF *sc = extract_source_component(afs[af_count - 1]);
		printf("source component size:%d\n", sc->size);
		// print_argumentation_framework(sc);

		bool *ce = NULL;
		// ce = dc_co(sc, -1);

		// look for a non-empty complete extension
		for (SIZE_TYPE i = 0; i < sc->size; ++i) {
			ce = dc_co(sc, i);
			if (ce != NULL) {
				break;
			}
		}
		if (ce == NULL)
			return(NULL);

		// printf("ce: ");
		SIZE_TYPE size_ce = 0;
		// determine size of the ce
		for (SIZE_TYPE i = 0; i < sc->size; ++i)
			if (ce[i]) {
				// printf("%d ", i);
				++size_ce;
			}
		// printf("\n");
		printf("size_ce: %d\n", size_ce);
		if (size_ce == 0) {
		// if (ce == NULL) {
			bool *remove = calloc(afs[af_count - 1]->size, sizeof(bool));
			assert(remove != NULL);
			for (SIZE_TYPE i = 0; i < sc->size; ++i) {
				remove[sc->mapping[i]] = true;
			}
			++af_count;
			AF **tmp = realloc(afs, (af_count) * sizeof(AF*));
			assert(tmp != NULL);
			afs = tmp;
			afs[af_count - 1] = extract_residual_framework(afs[af_count - 2], remove, sc->size);
			printf("residual size: %d\n", afs[af_count - 1]->size);
			free(remove);
			// TODO: update P
		}
		else {
			// printf("ce: ");
			SIZE_TYPE size_ce = 0;
			// determine size of the ce
			for (SIZE_TYPE i = 0; i < sc->size; ++i)
				if (ce[i]) {
					// printf("%d ", i);
					++size_ce;
				}
			// printf("\n");
			// printf("size_ce: %d\n", size_ce);

			Stack *update = new_stack();
			// Push the arguments in ce to the stack.
			// printf("stack: ");
			for (SIZE_TYPE i = 0; i < sc->size; ++i)
				if (ce[i]) {
					push(update, new_stack_element_int(sc->mapping[i]));
					// printf("%d ", sc->mapping[i]);
				}
			// printf("\n");

			// Push the unattacked arguments to the stack. They are defended by every set.
			AF *af_t = transpose_argumentation_framework(afs[af_count - 1]);
			for (SIZE_TYPE i = 0; i < af_t->size; ++i) {
				if (af_t->list_sizes[i] == 0) {
					push(update, new_stack_element_int(sc->mapping[i]));
				}
			}
			// Compute closure in afs[af_count], not in the sc
			Node *n = create_node(afs[af_count - 1]->size);
			memcpy(n->unattacked_attackers_count, af_t->list_sizes, af_t->size * sizeof(SIZE_TYPE));
			memcpy(n->not_attacker_of_current_count, af_t->list_sizes, af_t->size * sizeof(SIZE_TYPE));
			memcpy(n->allowed_attackers_count, af_t->list_sizes, af_t->size * sizeof(SIZE_TYPE));
			n = pseudo_complete(update, n, afs[af_count - 1], af_t);
			free_stack(update);
			// prepare arguments to be removed
			SIZE_TYPE size_remove = 0;
			// printf("n->set: ");
			bool *remove = calloc(afs[af_count - 1]->size, sizeof(bool));
			assert(remove != NULL);
			// arguments in the closure will be removed from afs[af_count]
			for (SIZE_TYPE i = 0; i < afs[af_count - 1]->size; ++i) {
				/*
				if (n->set[i] || n->victims[i]) {
					++size_remove;
					remove[i] = true;
					printf("%d ", i);
				}
				*/
				if (n->set[i]) {
					++size_remove;
					remove[i] = true;
					SIZE_TYPE back_mapped = i;
					for (int j = af_count - 1; j > 0; --j) {
						back_mapped = afs[j]->mapping[back_mapped];
					}
					printf("**%d**", back_mapped + 1);
				}
				if (n->victims[i]) {
					++size_remove;
					remove[i] = true;
				}
			}
			// also the victims will be removed

			printf("\nsize_s:%d\n", size_remove);
			delete_node(n);
			++af_count;
			AF **tmp = realloc(afs, af_count  * sizeof(AF*));
			assert(tmp != NULL);
			afs = tmp;
			afs[af_count - 1] = extract_residual_framework(afs[af_count - 2], remove, size_remove);
			free(remove);
		}
		free_argumentation_framework(sc);
	}
	/*
	Stack *update = new_stack();
	// Push the unattacked arguments to the stack. They are defended by every set.
	for (SIZE_TYPE i = 0; i < sc_t->size; ++i) {
		if (sc_t->list_sizes[i] == 0) {
			push(update, new_stack_element_int(i));
		}
	}
	// First closure.
	Node *n = create_node(sc->size);
	n = pseudo_complete(update, n, sc, sc_t);
	*/

	/*
	bool *args = calloc(sc->size, sizeof(bool));
	assert(args != NULL);
	args[0] = 1;
	AF *rf = extract_residual_framework(sc, args, 1);

	print_argumentation_framework(rf);
	printf("**\n");
	for (SIZE_TYPE i = 0; i < rf->size; ++i) {
		printf("%d -> %d\n", i+1, sc->mapping[rf->mapping[i]]+1);
	}
	*/

	return(NULL);
}