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
#include <math.h>
#include <string.h>

#include "af.h"
#include "../utils/stack.h"
#include "../utils/array_list.h"

AF* create_argumentation_framework(SIZE_TYPE size) {
	AF* af = calloc(1, sizeof(AF));
	assert(af != NULL);

	af->size = size;

	af->lists = calloc(af->size, sizeof(ARG_TYPE*));
	assert(af->lists != NULL);
	af->list_sizes = calloc(af->size, sizeof(SIZE_TYPE));
	assert(af->list_sizes != NULL);

	af->offsets = NULL;

	return(af);
}

int free_argumentation_framework(AF* af) {
	int freed_bytes = 0;
	for (SIZE_TYPE i = 0; i < af->size; ++i) {
		// freed_bytes += list_free(af->lists[i]);
		free(af->lists[i]);
		freed_bytes += af->list_sizes[i] * sizeof(ARG_TYPE);
	}
	free(af->lists);
	freed_bytes += af->size * sizeof(ARG_TYPE*);
	// freed_bytes += af->size * sizeof(List*);
	free(af->list_sizes);
	freed_bytes += af->size * sizeof(SIZE_TYPE);
	free(af->offsets);
	freed_bytes += af->size * sizeof(int);
	free(af);
	freed_bytes += sizeof(AF);
	return(freed_bytes);
}

void print_argumentation_framework(AF* af) {
	for (SIZE_TYPE i = 0; i < af->size; ++i) {
		// print_list(af->lists[i]);
		for (SIZE_TYPE j = 0; j < af->list_sizes[i]; ++j)
			printf("%d %d\n", i + 1, af->lists[i][j] + 1);
	}
}


AF* transpose_argumentation_framework(AF *af) {
	AF* t_af = create_argumentation_framework(af->size);

	t_af->size = af->size;

	for (SIZE_TYPE i = 0; i < af->size; ++i)
		for (SIZE_TYPE j = 0; j < af->list_sizes[i]; ++j) {
			add_attack(t_af, af->lists[i][j], i);
		}

	return(t_af);
}

bool is_set_self_defending(AF* attacks, AF* attacked_by, ArrayList* s) {
	bool* victims = calloc(attacks->size, sizeof(bool));
	assert(victims != NULL);
	for (SIZE_TYPE i = 0; i < s->size; ++i)
		for (SIZE_TYPE j = 0; j < attacks->list_sizes[s->elements[i]]; ++j) {
			victims[attacks->lists[s->elements[i]][j]] = true;
		}

	for (SIZE_TYPE i = 0; i < s->size; ++i)
		for (SIZE_TYPE j = 0; j < attacked_by->list_sizes[s->elements[i]]; ++j) {
			if (!victims[attacked_by->lists[s->elements[i]][j]]) {
				free(victims);
				return(false);
			}
		}
	free(victims);
	return(true);
}

// Returns true if s is conflict-free
bool is_set_conflict_free(AF* attacks, ArrayList* s) {
	bool* victims = calloc(attacks->size, sizeof(bool));
	assert(victims != NULL);

	for (SIZE_TYPE i = 0; i < s->size; ++i) {
		for (SIZE_TYPE j = 0; j < attacks->list_sizes[s->elements[i]]; ++j) {
			victims[attacks->lists[s->elements[i]][j]] = true;
		}
	}

	for (SIZE_TYPE i = 0; i < s->size; ++i)
		if (victims[s->elements[i]]) {
			free(victims);
			return(false);
		}

	free(victims);
	return(true);
}

// Add an attack from argument at index i to argument at index j
bool add_attack(AF* af, ARG_TYPE i, ARG_TYPE j) {
	ARG_TYPE* tmp = realloc(af->lists[i], (af->list_sizes[i] + 1) * sizeof(ARG_TYPE));
	assert(tmp != NULL);
	af->lists[i] = tmp;
	af->lists[i][af->list_sizes[i]] = j;
	++af->list_sizes[i];
	return(true);

}

bool check_set_defends_arg(ArrayList *s, ARG_TYPE a, AF *attacks, AF *attacked_by) {
	bool* victims = calloc(attacks->size, sizeof(bool));
	assert(victims != NULL);
	for (SIZE_TYPE i = 0; i < s->size; ++i)
		for (SIZE_TYPE j = 0; j < attacks->list_sizes[s->elements[i]]; ++j) {
			victims[attacks->lists[s->elements[i]][j]] = true;
		}

	for (SIZE_TYPE i = 0; i < attacked_by->list_sizes[a]; ++i)
		if (!victims[attacked_by->lists[a][i]]) {
			free(victims);
			return(false);
		}

	free(victims);
	return(true);
}

bool is_set_stable(AF *attacks, ArrayList *s) {
	bool* victims = calloc(attacks->size, sizeof(bool));
	assert(victims != NULL);

	bool* s_bv = calloc(attacks->size, sizeof(bool));
	assert(s_bv != NULL);

	for (SIZE_TYPE i = 0; i < s->size; ++i) {
		s_bv[s->elements[i]] = true;
		for (SIZE_TYPE j = 0; j < attacks->list_sizes[s->elements[i]]; ++j) {
			victims[attacks->lists[s->elements[i]][j]] = true;
		}
	}

	// is set conflict-free?
	for (SIZE_TYPE i = 0; i < s->size; ++i)
		if (victims[s->elements[i]]) {
			free(victims);
			free(s_bv);
			return(false);
		}

	// is set self-defending?
	AF* attacked_by = transpose_argumentation_framework(attacks);
	for (SIZE_TYPE i = 0; i < s->size; ++i)
		for (SIZE_TYPE j = 0; j < attacked_by->list_sizes[s->elements[i]]; ++j) {
			if (!victims[attacked_by->lists[s->elements[i]][j]]) {
				free(victims);
				free(s_bv);
				return(false);
			}
		}

	// check if s attacks everything outside
	for (SIZE_TYPE i = 0; i < attacks->size; ++i)
		if (!s_bv[i] && !victims[i]) {
			free(victims);
			free(s_bv);
			return(false);
		}

	free(victims);
	free(s_bv);
	return(true);
}

bool is_set_complete(AF* af, ArrayList* s) {
	AF* af_t = transpose_argumentation_framework(af);
	bool admissible = is_set_conflict_free(af, s) && is_set_self_defending(af, af_t, s);
	if (!admissible) {
		free_argumentation_framework(af_t);
		return(0);
	}

	ArrayList* closure = list_duplicate(s);
	bool* closure_bv = calloc(af->size, sizeof(bool));
	assert(closure_bv != NULL);
	for (SIZE_TYPE i = 0; i < closure->size; ++i) {
		closure_bv[closure->elements[i]] = true;
	}
	bool updated = false;
	do {
		updated = false;
		for (SIZE_TYPE i = 0; i < af->size; ++i) {
			if (!closure_bv[i] && check_set_defends_arg(closure, i, af, af_t)) {
				list_add(i, closure);
				closure_bv[i] = true;
				updated = true;
			}
		}
	} while (updated);

	bool equal = is_list_equal(s, closure);

	free_argumentation_framework(af_t);
	free(closure_bv);
	list_free(closure);

	return(equal);
}

bool is_set_admissible(AF* af, ArrayList* s) {
	AF* af_t = transpose_argumentation_framework(af);
	bool admissible = is_set_conflict_free(af, s) && is_set_self_defending(af, af_t, s);
	free_argumentation_framework(af_t);
	return(admissible);
}

int comp(const void *a, const void *b) {
	return (*(int *)a - *(int *)b);
}

// remove the arguments args and their victims from the framework af
// return a new framework
AF* extract_residual_framework(AF* af, ARG_TYPE *args, int arg_count) {
	int *offsets = calloc(af->size, sizeof(int));
	assert(offsets != NULL);

	// number of removed arguments
	int removed_count = arg_count;
	// array of the removed arguments
	ARG_TYPE *removed = calloc(arg_count, sizeof(ARG_TYPE));
	assert(removed != NULL);
	memcpy(removed, args, arg_count * sizeof(ARG_TYPE));

	for (SIZE_TYPE i = 0; i < arg_count; ++i) {
		// mark the removed arguments with -1
		offsets[args[i]] = -1;
		// and also their victims
		for (SIZE_TYPE j = 0; j < af->list_sizes[args[i]]; ++j) {
			// if not already marked as removed
			if (offsets[af->lists[args[i]][j]] != -1) {
				offsets[af->lists[args[i]][j]] = -1;
				ARG_TYPE *tmp = realloc(removed, sizeof(ARG_TYPE) * (removed_count+1));
				assert(tmp != NULL);
				removed = tmp;
				// add the victim to the removed
				removed[removed_count] = af->lists[args[i]][j];
				++removed_count;
			}
		}
	}
	// sort the removed arguments and their victims
	qsort(removed, removed_count, sizeof(ARG_TYPE), comp);

	// update the offsets except for the last one
	SIZE_TYPE start, end = removed[0], offset = 0;
	for (int i = 0; i < removed_count - 1; ++i) {
		start = removed[i] + 1;
		end = removed[i+1];
		offset = i + 1;
		for (SIZE_TYPE j = start; j < end; ++j)
			offsets[j] = offset;
	}
	// update the last offset
	start = end + 1;
	end = af->size;
	++offset;
	for (SIZE_TYPE j = start; j < end; ++j)
		offsets[j] = offset;

	// free the removed. not used below here
	free(removed);

	AF *rf = create_argumentation_framework(af->size - removed_count);
	rf->offsets = calloc(af->size - removed_count, sizeof(int));
	assert(rf->offsets != NULL);

	SIZE_TYPE index = 0;
	for (SIZE_TYPE i = 0; i < af->size; ++i) {
		// if argument is not removed from the framework
		if (offsets[i] != -1) {
			// add the not removed victims to the residual framework
			for (SIZE_TYPE j = 0; j < af->list_sizes[i]; ++j)
				if (offsets[af->lists[i][j]] != -1) {
					add_attack(rf, i - offsets[i], af->lists[i][j] - offsets[af->lists[i][j]]);
				}
			// update rf->offsets
			if (af->offsets == NULL) {
				// if af->offsets is null, then af is the original framework
				rf->offsets[index] = offsets[i];
			}
			else {
				// otherwise af is a residual framework, from a previous step
				rf->offsets[index] = offsets[i] + af->offsets[i];
			}
			++index;
		}
	}
	free(offsets);

	return(rf);
}

int *extract_sccs(AF *af, AF *af_t) {
	bool *visited = calloc(af->size, sizeof(bool));
	assert(visited != NULL);

	// stack for the dfs
	Stack *s = new_stack();
	push(s, new_stack_element_int(0));

	// stack for the order of vertices
	Stack *l = new_stack();

	// dfs forwards
	SIZE_TYPE a = -1;
	while ((a = pop_int(s)) != -1) {
		visited[a] = true;
		push(l, new_stack_element_int(a));
		for (SIZE_TYPE i = 0; i < af->list_sizes[a]; ++i) {
			if (!visited[af->lists[a][i]]) {
				push(s, new_stack_element_int(af->lists[a][i]));
			}
		}
	}
	free_stack(s);

	// the reverse order
	Stack *l_rev = new_stack();
	while ((a = pop_int(l)) != -1) {
		push(l_rev, new_stack_element_int(a));
	}
	free_stack(l);

	// reset visited for the backwards dfs
	memset(visited, 0, af->size * sizeof(bool));

	int *components = calloc(af->size, sizeof(int));
	assert(components != NULL);
	memset(components, -1, af->size * sizeof(int));

	// dfs backwards
	// int component_count = 0;
	while ((a = pop_int(l_rev)) != -1) {
		visited[a] = true;
		// if a has not yet been assigned to a component
		if (components[a] == -1) {
			components[a] = a;
			// ++component_count;
		}
		for (SIZE_TYPE i = 0; i < af_t->list_sizes[a]; ++i) {
			if (!visited[af_t->lists[a][i]]) {
				push(l_rev, new_stack_element_int(af_t->lists[a][i]));
				components[af_t->lists[a][i]] = components[a];
			}
		}
	}
	free(visited);
	free_stack(l_rev);

	/*
	printf("%d components: ", component_count);
	for (SIZE_TYPE i = 0; i < af->size; ++i)
		printf("%d ", components[i]);
	printf("\n\n");
	*/
	return(components);
}