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
#include "../utils/list.h"


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
inline bool check_arg_attacks_set(AF* af, ARG_TYPE arg, List* s) {
	for (SIZE_TYPE i; i < s->size; ++i)
		if (check_arg_attacks_arg(af, arg, s->elements[i]))
			return(true);
	return(false);
}

// Returns true if s attacks arg
inline bool check_set_attacks_arg(AF* af, List* s, ARG_TYPE arg) {
	for (SIZE_TYPE i; i < s->size; ++i)
		if (check_arg_attacks_arg(af, s->elements[i], arg))
			return(true);
	return(false);
}
// Returns true if s is consistent
// TODO: Optimize!
inline bool is_set_consistent(AF* attacks, List* s) {
	/*
	for (SIZE_TYPE i = 0; i < s->size; ++i)
		if (check_arg_attacks_set(af, s->elements[i], s) || check_set_attacks_arg(af, s, s->elements[i]))
			return(true);
	return(false);
	*/
	for (SIZE_TYPE i = 0; i < s->size; ++i)
		for (SIZE_TYPE j = 0; j < s->size; ++j)
			for (SIZE_TYPE k = 0; k < attacks->list_sizes[s->elements[j]]; ++k)
				if (attacks->lists[s->elements[j]][k] == i)
					return(false);
	return(true);
}

bool is_set_self_defending(AF* attacks, AF* attacked_by, List* s);

// Check if argument i attacks argument j
// Here i and j start from "0"
// #define CHECK_ARG_ATTACKS_ARG(af,i,j)		TEST_BIT(af->graph[i],j)

// Check if arg attacks bs
// Return 1 if yes, 0 otherwise
// #define CHECK_ARG_ATTACKS_SET(af,arg,bs)	(!is_bitset_intersection_empty(af->graph[arg],bs))

// Check if set s attacks argument arg
// Return 1 if yes, 0 otherwise
// inline char check_set_attacks_arg(AF* af, BitSet* s, int arg) {
// 	for (SIZE_TYPE i = 0; i < af->size; ++i)
// 		if (TEST_BIT(s,i) && CHECK_ARG_ATTACKS_ARG(af, i, arg))
//			return(1);
//	return(0);
//}

// Check if set s defends argument arg
// Return 1 if yes, 0 otherwise
//inline char check_set_defends_arg(AF* af, BitSet* s, int arg) {
//	// Check if s attacks all attackers of arg
//	for (SIZE_TYPE i = 0; i < af->size; ++i)
//		if (CHECK_ARG_ATTACKS_ARG(af, i, arg) && !check_set_attacks_arg(af, s, i))
//			return(0);
//	return(1);
//}

/*
int is_conflict_free(Context* attacks, BitSet* x) {
	BitSet* x_attacks = create_bitset(attacks->size);
	BitSet* r = create_bitset(attacks->size);
	int i;
	for (i = 0; i < attacks->size; ++i) {
		if (TEST_BIT(x, i))
			bitset_union(x_attacks, attacks->a[i], x_attacks);
	}
	bitset_intersection(x, x_attacks, r);
	if (bitset_is_emptyset(r))
		return(1);
	return(0);
}
*/

// TODO: compare to the above. Which is more efficient?

//inline char is_set_conflict_free(AF* af, BitSet* s) {
//	for (SIZE_TYPE i = 0; i < af->size; ++i)
//		if (TEST_BIT(s, i))
//			for (SIZE_TYPE j = i; j < af->size; ++j)
//				if (TEST_BIT(s, j) && (CHECK_ARG_ATTACKS_ARG(af, i, j) || CHECK_ARG_ATTACKS_ARG(af, j, i)))
//					return(0);
//	return(1);
//}
/*
// TODO: Compare to above
inline char is_set_conflict_free(AF* af, BitSet* s) {
	for (SIZE_TYPE i = 0; i < af->size; ++i)
		if (TEST_BIT(s, i) && CHECK_ARG_ATTACKS_SET(af, i, s))
			return(0);
	return(1);
}
*/

// Compute attackers of set s, store in r.
// attacked_by: the transposed framework
// s: the given bitset
// r: result bitset containing attackers of s
//static inline void get_attackers(AF* attacked_by, BitSet* s, BitSet* r) {
//	reset_bitset(r);
//	for (SIZE_TYPE i = 0; i < attacked_by->size; ++i) {
//		if (TEST_BIT(s, i))
//			bitset_union(r, attacked_by->graph[i], r);
//	}
//}


//static inline void get_true_attackers(AF* af, BitSet* s, BitSet* r) {
//	reset_bitset(r);
//	for (SIZE_TYPE i = 0; i < af->size; ++i) {
//		if (!is_bitset_intersection_empty(af->graph[i], s)) {
//			SET_BIT(r, i);
//		}
//	}
//}

// Compute victims (attacked arguments) of a set s, store in r
// attacks: the attacks framework
// s: the given bitset
// r: result bitset containing arguments attacked by s
//static inline void get_victims(AF* attacks, BitSet* s, BitSet* r) {
//	reset_bitset(r);
//	for (SIZE_TYPE i = 0; i < attacks->size; ++i) {
//		if (TEST_BIT(s, i))
//			bitset_union(r, attacks->graph[i], r);
//	}
//}

// Compute common victims of the arguments in s (arguments attacked by all elements of s)
// (up-arrow in FCA terms) Put the result in r
// inline void up_arrow(AF* af, BitSet* s, BitSet* r) {
//static inline void up_arrow(AF* af, BitSet* s, BitSet* r) {
//	// First fill r
//	// TODO: Improve efficiency?
//	set_bitset(r);
//
//	for (SIZE_TYPE i = 0; i < af->size; ++i)
//		if (TEST_BIT(s, i))
//			bitset_intersection(r, af->graph[i], r);
//}

// Compute total attackers of s (arguments attacking all elements of s)
// (down-arrow in FCA terms) Put the result in r
// inline void get_total_attackers(AF* af, BitSet* s, BitSet* r) {
//static inline void down_arrow(AF* af, BitSet* s, BitSet* r) {
//	// TODO: Improve efficiency?
//	reset_bitset(r);
//
//	for (SIZE_TYPE i = 0; i < af->size; ++i)
//		if (bitset_is_subset(s, af->graph[i]))
//			SET_BIT(r, i);
//}

// Compute common victims of the total attackers of s
// (arguments commonly attacked by the arguments that attack all elements of s.
// In FCA: down-up-arrow closure operator on the formal context)
// inline void get_common_victims_of_total_attackers(AF* af, BitSet* s, BitSet* r) {
//static inline void down_up_arrow(AF* af, BitSet* s, BitSet* r) {
//	// First fill //r
//	// TODO: Improve efficiency?
//	set_bitset(r);
//
//	for (SIZE_TYPE i = 0; i < af->size; ++i) {
//		if (bitset_is_subset(s, af->graph[i])) {
//			bitset_intersection(r, af->graph[i], r);
//		}
//	}
//}

AF* complement_argumentation_framework(AF *af );

AF* transpose_argumentation_framework(AF *af);

AF* create_conflict_framework(AF* af);	// make it undirected

//PAF* project_argumentation_framework(AF *af, BitSet* mask);

//BitSet* project_back(BitSet* bs, PAF* paf, SIZE_TYPE base_size);

// swap the order of arguments i and j
// void swap_arguments(AF* af, SIZE_TYPE i, SIZE_TYPE j);

// Map indices of bitset s according to the mapping, return the new bitset
// BitSet *map_indices(BitSet *s, int *mapping);

#endif /* AF_AF_H_ */
