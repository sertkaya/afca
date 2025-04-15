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
#include <stdbool.h>

#include "af.h"

#include <string.h>

#include "../utils/stack.h"
#include "../utils/linked_list.h"
#include "../utils/array_list.h"
#include "../utils/implication.h"
#include "../algorithms/next-closure/complete.h"

AF* create_argumentation_framework(SIZE_TYPE size) {
	AF* af = calloc(1, sizeof(AF));
	assert(af != NULL);

	af->size = size;

	// af->lists = calloc(af->size, sizeof(List*));
	af->lists = calloc(af->size, sizeof(ARG_TYPE*));
	assert(af->lists != NULL);
	af->list_sizes = calloc(af->size, sizeof(SIZE_TYPE));
	assert(af->list_sizes != NULL);

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

int compare_argument(const void *arg1, const void *arg2) {
	if (arg1 < arg2)
		return(1);
	if (arg1 > arg2)
		return(-1);
	return(0);
}

AF* transpose_argumentation_framework(AF *af) {
	AF* t_af = create_argumentation_framework(af->size);

	t_af->size = af->size;

	for (SIZE_TYPE i = 0; i < af->size; ++i)
		// for (SIZE_TYPE j = 0; j < af->lists[i]->size; ++j)
		for (SIZE_TYPE j = 0; j < af->list_sizes[i]; ++j) {
			// ADD_ATTACK(t_af, j + 1, i + 1);
			add_attack(t_af, af->lists[i][j], i);
		}

	/*
	// Sort the adjacency lists
	for (SIZE_TYPE i = 0; i < af->size; ++i)
		// qsort(t_af->lists[i]->elements, t_af->lists[i]->size, sizeof(ARG_TYPE), cmp);
		qsort(t_af->lists[i], t_af->list_sizes[i], sizeof(ARG_TYPE), compare_argument);
		*/

	return(t_af);
}

bool is_set_self_defending(AF* attacks, AF* attacked_by, ArrayList* s) {
	bool* victims = calloc(attacks->size, sizeof(bool));
	assert(victims != NULL);
	for (SIZE_TYPE i = 0; i < s->size; ++i)
		for (SIZE_TYPE j = 0; j < attacks->list_sizes[s->elements[i]]; ++j) {
			victims[attacks->lists[s->elements[i]][j]] = true;
		}

	bool* attackers = calloc(attacks->size, sizeof(bool));
	assert(attackers != NULL);
	for (SIZE_TYPE i = 0; i < s->size; ++i)
		for (SIZE_TYPE j = 0; j < attacked_by->list_sizes[s->elements[i]]; ++j) {
			attackers[attacked_by->lists[s->elements[i]][j]] = true;
		}

	for (SIZE_TYPE i = 0; i < attacks->size; ++i)
		if (attackers[i] && !victims[i]) {
			free(victims);
			free(attackers);
			return(false);
		}

	free(victims);
	free(attackers);
	return(true);
}

Subgraph* extract_subgraph_backwards(AF* af, AF* af_t, ARG_TYPE argument) {

	bool* visited = calloc(af_t->size, sizeof(bool));
	assert(visited != NULL);
	for (SIZE_TYPE i = 0; i < af_t->size; ++i)
		visited[i] = false;

	// find those nodes from which the argument is reachable, put them into visited
	ARG_TYPE a = argument;
	SIZE_TYPE subgraph_size = 0;

	Stack s;
	init_stack(&s);
	// push(&s, a);
	push(&s, new_stack_element_int(a));
	// while ((a = pop(&s)) != -1) {
	while ((a = pop_int(&s)) != -1) {
		if  (!visited[a]) {
			visited[a] = true;
			++subgraph_size;
		}
		for (SIZE_TYPE i = 0; i < af_t->list_sizes[a]; ++i)
			if (!visited[af_t->lists[a][i]]) { // && (af_t->lists[a][i] != a)) {
				// push(&s, af_t->lists[a][i]);
				push(&s, new_stack_element_int(af_t->lists[a][i]));
			}
	}

	// create a mapping from indices of the subgraph to
	// the indices of af
	ARG_TYPE* mapping_subgraph_af = calloc(subgraph_size, sizeof(ARG_TYPE));
	assert(mapping_subgraph_af != NULL);

	// create a mapping from indices of af to the
	// indices of subgraph
	ARG_TYPE* mapping_af_subgraph = calloc(af->size, sizeof(ARG_TYPE));
	assert(mapping_af_subgraph != NULL);
	// initially every index is mapped to itself
	for (SIZE_TYPE i = 0; i < af->size; ++i)
		mapping_af_subgraph[i] = i;

	SIZE_TYPE subgraph_index = 0;

	for (SIZE_TYPE i = 0; i < af->size; ++i)
		if (visited[i]) {
			mapping_af_subgraph[i] = subgraph_index;
			mapping_subgraph_af[subgraph_index++] = i;
		}

	// create the subgraph
	Subgraph* subgraph = calloc(1, sizeof(Subgraph));
	assert(subgraph != NULL);
	subgraph->af = create_argumentation_framework(subgraph_size);
	subgraph->mapping_from_subgraph = mapping_subgraph_af;
	subgraph->mapping_to_subgraph = mapping_af_subgraph;

	int attack_count = 0;
	for (SIZE_TYPE i = 0; i < af->size; ++i) {
		if (visited[i]) {
			for (SIZE_TYPE j = 0; j < af->list_sizes[i]; ++j) {
				// add_attack(subgraph->af, mapping_af_subgraph[af->lists[i][j]], mapping_af_subgraph[i]);
				if (visited[af->lists[i][j]])
					add_attack(subgraph->af, mapping_af_subgraph[i], mapping_af_subgraph[af->lists[i][j]]);
			}
		}
	}

	free(visited);
	// free(mapping_af_subgraph);
	// free(mapping_subgraph_af);
	return(subgraph);
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
// Returns true if arg_1 attacks arg_2
// Assumes that the adjacency lists are sorted!
bool check_arg_attacks_arg(AF* af, ARG_TYPE arg_1, ARG_TYPE arg_2) {
	for (SIZE_TYPE i = 0; i < af->list_sizes[arg_1]; ++i)
		if (af->lists[arg_1][i] == arg_2) {
			return(true);
		}
	return(false);
}

// Returns true if s attacks arg
bool check_set_attacks_arg(AF* af, ArrayList* s, ARG_TYPE arg) {
	for (SIZE_TYPE i = 0; i < s->size; ++i)
		if (check_arg_attacks_arg(af, s->elements[i], arg))
			return(true);
	return(false);
}

// Returns true if arg attacks s
bool check_arg_attacks_set(AF* af, ARG_TYPE arg, ArrayList* s) {
	for (SIZE_TYPE i = 0; i < s->size; ++i)
		if (check_arg_attacks_arg(af, arg, s->elements[i]))
			return(true);
	return(false);
}

// swap arguments a1 and a2 in the framework af
void swap_arguments(AF* af, ARG_TYPE a1, ARG_TYPE a2) {
	// pointer to list a1
	ARG_TYPE *list_a1 = af->lists[a1];
	// size of list a1
	SIZE_TYPE list_size_a1 = af->list_sizes[a1];

	// swap list sizes
	af->list_sizes[a1] = af->list_sizes[a2];
	af->list_sizes[a2] = list_size_a1;

	// swap lists
	af->lists[a1] = af->lists[a2];
	af->lists[a2] = list_a1;

	// do swapping in the whole graph
	for (SIZE_TYPE i = 0; i < af->size; ++i)
		for (SIZE_TYPE j = 0; j < af->list_sizes[i]; ++j) {
			if (af->lists[i][j] == a1)
				af->lists[i][j] = a2;
			else if (af->lists[i][j] == a2)
				af->lists[i][j] = a1;
		}
}

bool is_set_complete(AF* af, ArrayList* s) {
	AF* af_t = transpose_argumentation_framework(af);
	bool admissible = is_set_conflict_free(af, s) && is_set_self_defending(af, af_t, s);
	if (!admissible) {
		free_argumentation_framework(af_t);
		return(0);
	}

	ArrayList* closure = list_create();
	bool* closure_bv = calloc(af->size, sizeof(bool));
	assert(closure_bv != NULL);
	closure_semi_complete(af, af_t, s, closure, closure_bv);

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

AF* apply_mapping(AF* af, ARG_TYPE* mapping) {
	AF* mapped_af = create_argumentation_framework(af->size);

	mapped_af->size = af->size;
	for (SIZE_TYPE i = 0; i < af->size; ++i) {
		ARG_TYPE mapped_i = mapping[i];
		mapped_af->list_sizes[i] = af->list_sizes[mapped_i];
		mapped_af->lists[i] = calloc(mapped_af->list_sizes[i], sizeof(ARG_TYPE));
		assert(mapped_af->lists[i] != NULL);
		memcpy(mapped_af->lists[i], af->lists[mapped_i], mapped_af->list_sizes[i] * sizeof(ARG_TYPE));
	}

	for (SIZE_TYPE i = 0; i < mapped_af->size; ++i) {
		ARG_TYPE mapped_i = mapping[i];
		for (SIZE_TYPE j = 0; j < mapped_af->list_sizes[i]; ++j) {
			mapped_af->lists[i][j] = mapping[af->lists[mapped_i][j]];
		}
	}

	return(mapped_af);
}

struct argument_score_pair {
	ARG_TYPE arg;
	SIZE_TYPE victim_count;
};

int compare_arguments(const void *v1, const void *v2) {
	if ((((struct argument_score_pair*) v1)-> victim_count) < (((struct argument_score_pair*) v2)-> victim_count))
		return(-1);
	else if ((((struct argument_score_pair*) v1)-> victim_count) > (((struct argument_score_pair*) v2)-> victim_count))
		return(1);
	else
		return(0);
}

void sort_adjacency_lists(AF *af, AF *af_t) {
	for (SIZE_TYPE i = 0; i < af_t->size; ++i) {
		struct argument_score_pair *pairs = calloc(af_t->list_sizes[i], sizeof(struct argument_score_pair));
		assert(pairs != NULL);
		for (SIZE_TYPE j = 0; j < af_t->list_sizes[i]; ++j) {
			pairs[j].arg = af_t->lists[i][j];
			pairs[j].victim_count = af->list_sizes[af_t->lists[i][j]];
		}
		qsort(pairs, af_t->list_sizes[i], sizeof(struct argument_score_pair), compare_arguments);
		for (SIZE_TYPE j = 0; j < af_t->list_sizes[i]; ++j) {
			af_t->lists[i][j] = pairs[j].arg;
		}
		free(pairs);
	}
}

void print_conflicts_matrix(bool **conflicts, AF* af) {
	for (SIZE_TYPE i = 0; i < af->size; ++i) {
		for (SIZE_TYPE j = 0; j < af->size; ++j)
			if (conflicts[i][j])
				printf("1");
			else
				printf("0");
		printf("\n");
	}
}

AF *create_conflicts_graph(AF *af, AF *af_t) {
	// ListNode *implications = create_implications(af, af_t);
	AF *conflicts_af = create_argumentation_framework(af->size);
	for (SIZE_TYPE i = 0; i < af->size; ++i) {
		for (SIZE_TYPE j = 0; j < af->list_sizes[i]; ++j) {
			add_attack(conflicts_af, i, af->lists[i][j]);
			add_attack(conflicts_af, af->lists[i][j], i);
		}
		// close(i, implications, conflicts_matrix[i]);
	}
	return(conflicts_af);
}