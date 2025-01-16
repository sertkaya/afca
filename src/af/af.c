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
#include <inttypes.h>
#include <math.h>
#include <inttypes.h>
#include <stdbool.h>

#include "af.h"
#include "../bitset/bitset.h"
#include "../utils/timer.h"
#include "../utils/stack.h"

AF* create_argumentation_framework(SIZE_TYPE size) {
	AF* af = calloc(1, sizeof(AF));
	assert(af != NULL);

	af->size = size;

	// af->lists = calloc(af->size, sizeof(List*));
	af->lists = calloc(af->size, sizeof(ARG_TYPE*));
	assert(af->lists != NULL);
	af->list_sizes = calloc(af->size, sizeof(SIZE_TYPE));
	assert(af->list_sizes != NULL);

	/*
	for (SIZE_TYPE i = 0; i < size; ++i) {
		af->lists[i] = list_create();
	}
	*/

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


/*
int free_projected_argumentation_framework(PAF* paf) {
	free(paf->index_mapping);
	int freed_bytes = paf->af->size * sizeof(SIZE_TYPE);
	freed_bytes += free_argumentation_framework(paf->af);
	free(paf);
	return freed_bytes + sizeof(PAF);
}
*/


void print_argumentation_framework(AF* af) {
	for (SIZE_TYPE i = 0; i < af->size; ++i) {
		// print_list(af->lists[i]);
		for (SIZE_TYPE j = 0; j < af->list_sizes[i]; ++j)
			printf("%d %d\n", i + 1, af->lists[i][j] + 1);
	}
}

/*
int cmp(const void* arg_1, const void* arg_2) {
	if ((*(ARG_TYPE*) arg_1) < *((ARG_TYPE*) arg_2))
		return(-1);
	if ((*(ARG_TYPE*) arg_1) > *((ARG_TYPE*) arg_2))
		return(1);
	return(0);
}
*/

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
		qsort(t_af->lists[i], t_af->list_sizes[i], sizeof(ARG_TYPE), cmp);
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
			victims[attacked_by->lists[s->elements[i]][j]] = true;
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

Subgraph* extract_subgraph_backwards(AF* af, ARG_TYPE argument) {
	Stack s;
	init_stack(&s);

	bool* visited = calloc(af->size, sizeof(bool));
	assert(visited != NULL);

	// find the nodes reachable from argument, put into visited
	SIZE_TYPE a = argument;
	SIZE_TYPE subgraph_size = 0;
	while (a != -1) {
		for (SIZE_TYPE i = 0; i < af->list_sizes[a]; ++i)
			if (!visited[af->lists[a][i]]) {
				push(&s, af->lists[a][i]);
				visited[af->lists[a][i]] = true;
				++subgraph_size;
			}
		a = pop(&s);
	}

	// create a mapping from indices of the subgraph to
	// the indices of af
	ARG_TYPE* mapping_subgraph_af = calloc(subgraph_size, sizeof(ARG_TYPE));
	assert(mapping_subgraph_af != NULL);
	// and a mapping from indices of af to the
	// indices of subgraph
	ARG_TYPE* mapping_af_subgraph = calloc(subgraph_size, sizeof(ARG_TYPE));
	assert(mapping_af_subgraph != NULL);
	SIZE_TYPE subgraph_index = 0;
	for (SIZE_TYPE i = 0; i < af->size; ++i)
		if (visited[i]) {
			mapping_af_subgraph[i] = subgraph_index;
			mapping_subgraph_af[subgraph_index++] = i;
		}

	// create the subgraph with backwards edges
	Subgraph* subgraph = calloc(1, sizeof(Subgraph));
	assert(subgraph != NULL);
	subgraph->af = create_argumentation_framework(subgraph_size);
	subgraph->mapping_from_subgraph = mapping_subgraph_af;
	subgraph->mapping_to_subgraph = mapping_af_subgraph;

	for (SIZE_TYPE i = 0; i < af->size; ++i) {
		if (visited[i]) {
			for (SIZE_TYPE j = 0; j < af->list_sizes[i]; ++j) {
				add_attack(subgraph->af, mapping_af_subgraph[af->lists[i][j]], mapping_af_subgraph[i]);
			}
		}
	}

	free(visited);
	// free(mapping_af_subgraph);
	// free(mapping_subgraph_af);
	return(subgraph);
}

/*
PAF* project_argumentation_framework(AF *af, bool* mask) {
	PAF *paf = calloc(1, sizeof(PAF));
	assert(paf != NULL);

	SIZE_TYPE mapping_size = 0;
	SIZE_TYPE mapped_index = 0;
	for (SIZE_TYPE i = 0; i < af->size; ++i) {
		if (mask[i]) {
			SIZE_TYPE* tmp = realloc(paf->index_mapping, (mapping_size + 1) * sizeof(SIZE_TYPE));
			assert(tmp != NULL);
			paf->index_mapping = tmp;

			++mapping_size;
		}
	}

    SIZE_TYPE size = 0;
	for (SIZE_TYPE i = 0; i < af->size; ++i)
		if (mask[i])
			++size;
	assert(size > 0);

    paf->index_mapping = calloc(size, sizeof(SIZE_TYPE));
    SIZE_TYPE j = 0;
    for (SIZE_TYPE i = 0; i < af->size; ++i) {
        if (mask[i]) {
            paf->index_mapping[j++] = i;
        }
    }

    paf->af = create_argumentation_framework(size);
    for (SIZE_TYPE i = 0; i < size; ++i) {
        for (SIZE_TYPE j = 0; j < size; ++j) {
            if (TEST_BIT(af->graph[paf->index_mapping[i]], paf->index_mapping[j])) {
				SET_BIT(paf->af->graph[i], j);
            }
        }
    }

	return paf;
}


BitSet* project_back(BitSet* bs, PAF* paf, SIZE_TYPE base_size) {
	BitSet* res = create_bitset(base_size);
	for (SIZE_TYPE i = 0; i < paf->af->size; ++i) {
		if (TEST_BIT(bs, i)) {
			SET_BIT(res, paf->index_mapping[i]);
		}
	}
	return res;
}


void swap_arguments(AF* af, SIZE_TYPE i, SIZE_TYPE j) {
	BitSet* iset = af->graph[i];
	af->graph[i] = af->graph[j];
	af->graph[j] = iset;

	for (SIZE_TYPE k = 0; k < af->size; ++k) {
		bool ibit = TEST_BIT(af->graph[k], i);
		if (TEST_BIT(af->graph[k], j)) {
			SET_BIT(af->graph[k], i);
		} else {
			RESET_BIT(af->graph[k], i);
		}
		if (ibit) {
			SET_BIT(af->graph[k], j);
		} else {
			RESET_BIT(af->graph[k], j);
		}
	}
}

// Map indices of bitset s according to the mapping.
// Return the new bitset
BitSet *map_indices(BitSet *s, int *mapping) {
  int i;
  BitSet* c = create_bitset(s->size);

  for (i = 0; i < s->size; ++i)
    if (TEST_BIT(s, i))
      SET_BIT(c, mapping[i]);

  return(c);
}
*/