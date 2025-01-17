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

#ifndef AF_AF_H_
#define AF_AF_H_

#include <stdbool.h>

#include "datatypes.h"
#include "../utils/array_list.h"


struct argumentation_framework {
	// Number of arguments
	SIZE_TYPE size;
    // Adjacency lists
	// List** lists;
    ARG_TYPE **lists;
    // Number of elements in each adjacency list
    SIZE_TYPE *list_sizes;
};

typedef struct argumentation_framework AF;

struct subgraph {
	AF* af;
	ARG_TYPE* mapping_to_subgraph;
	ARG_TYPE* mapping_from_subgraph;
};

typedef struct subgraph Subgraph;

struct projected_argumentation_framework {
	AF* af;
	// index_mapping[i] is the index of the ith argument of af in the original framework
	SIZE_TYPE* index_mapping;
};

typedef struct projected_argumentation_framework PAF;


// Create argumentation framework with the given number of arguments
AF* create_argumentation_framework(SIZE_TYPE size);

// Free the space allocated for af
// Return the number of bytes freed
int free_argumentation_framework(AF* af);

int free_projected_argumentation_framework(PAF* af);

void print_argumentation_framework(AF* af);

// Add an attack from argument at index i to argument at index j
// "i-1" and "j-1" since the arguments in the input file start with "1"
// #define ADD_ATTACK(af,i,j)		list_add(j-1, af->lists[i-1])

// Add an attack from argument at index i to argument at index j
inline bool add_attack(AF* af, ARG_TYPE i, ARG_TYPE j) {
	ARG_TYPE* tmp = realloc(af->lists[i], (af->list_sizes[i] + 1) * sizeof(ARG_TYPE));
	assert(tmp != NULL);
	af->lists[i] = tmp;
	af->lists[i][af->list_sizes[i]] = j;
	++af->list_sizes[i];
	return(true);

}

// Returns true if arg_1 attacks arg_2
// Assumes that the adjacency lists are sorted!
inline bool check_arg_attacks_arg(AF* af, ARG_TYPE arg_1, ARG_TYPE arg_2) {
	for (SIZE_TYPE i; i < af->list_sizes[arg_1]; ++i)
		if (af->lists[arg_1][i] == arg_2)
			return(true);
	return(false);
}

// Returns true if arg attacks s
inline bool check_arg_attacks_set(AF* af, ARG_TYPE arg, ArrayList* s) {
	for (SIZE_TYPE i; i < s->size; ++i)
		if (check_arg_attacks_arg(af, arg, s->elements[i]))
			return(true);
	return(false);
}

// Returns true if s attacks arg
inline bool check_set_attacks_arg(AF* af, ArrayList* s, ARG_TYPE arg) {
	for (SIZE_TYPE i; i < s->size; ++i)
		if (check_arg_attacks_arg(af, s->elements[i], arg))
			return(true);
	return(false);
}


bool is_set_self_defending(AF* attacks, AF* attacked_by, ArrayList* s);

Subgraph* extract_subgraph_backwards(AF* af, ARG_TYPE argument);

AF* complement_argumentation_framework(AF *af );

AF* transpose_argumentation_framework(AF *af);

AF* create_conflict_framework(AF* af);	// make it undirected

bool is_set_conflict_free(AF* attacks, ArrayList* s);

inline bool is_set_admissible(AF* af, ArrayList* s) {
	AF* af_t = transpose_argumentation_framework(af);
	bool admissible = is_set_conflict_free(af, s) && is_set_self_defending(af, af_t, s);
	free_argumentation_framework(af_t);
	return(admissible);
}

//PAF* project_argumentation_framework(AF *af, BitSet* mask);

//BitSet* project_back(BitSet* bs, PAF* paf, SIZE_TYPE base_size);

// swap the order of arguments i and j
// void swap_arguments(AF* af, SIZE_TYPE i, SIZE_TYPE j);

// Map indices of bitset s according to the mapping, return the new bitset
// BitSet *map_indices(BitSet *s, int *mapping);

#endif /* AF_AF_H_ */
