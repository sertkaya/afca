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

#include "../bitset/bitset.h"


struct argumentation_framework {
	// Number of arguments
	SIZE_TYPE size;
	// The adjacency matrix: array of bitsets
	BitSet **graph;
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
int free_argumentation_framework(AF *af);

int free_projected_argumentation_framework(PAF *af);

void print_argumentation_framework(AF *af);

// Add an attack from argument at index i to argument at index j
// "i-1" and "j-1" since the arguments in the input file start with "1"
#define ADD_ATTACK(af,i,j)		SET_BIT(af->graph[i-1],j-1)

// Check if argument i attacks argument j
// Here i and j start from "0"
#define CHECK_ARG_ATTACKS_ARG(af,i,j)		TEST_BIT(af->graph[i],j)

// Check if arg attacks bs
// Return 1 if yes, 0 otherwise
#define CHECK_ARG_ATTACKS_SET(af,arg,bs)	(!is_bitset_intersection_empty(af->graph[arg],bs))

// Check if set s attacks argument arg
// Return 1 if yes, 0 otherwise
inline char check_set_attacks_arg(AF* af, BitSet* s, int arg) {
	for (SIZE_TYPE i = 0; i < af->size; ++i)
		if (TEST_BIT(s,i) && CHECK_ARG_ATTACKS_ARG(af, i, arg))
			return(1);
	return(0);
}

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
inline char is_set_conflict_free(AF* af, BitSet* s) {
	for (SIZE_TYPE i = 0; i < af->size; ++i)
		if (TEST_BIT(s, i))
			for (SIZE_TYPE j = i; j < af->size; ++j)
				if (TEST_BIT(s, j) && (CHECK_ARG_ATTACKS_ARG(af, i, j) || CHECK_ARG_ATTACKS_ARG(af, j, i)))
					return(0);
	return(1);
}

// Compute common victims of the arguments in s (arguments attacked by all elements of s)
// (up-arrow in FCA terms) Put the result in r
// inline void up_arrow(AF* af, BitSet* s, BitSet* r) {
static inline void up_arrow(AF* af, BitSet* s, BitSet* r) {
	// First fill r
	// TODO: Improve efficiency?
	set_bitset(r);

	for (SIZE_TYPE i = 0; i < af->size; ++i)
		if (TEST_BIT(s, i))
			bitset_intersection(r, af->graph[i], r);
}

// Compute total attackers of s (arguments attacking all elements of s)
// (down-arrow in FCA terms) Put the result in r
// inline void get_total_attackers(AF* af, BitSet* s, BitSet* r) {
static inline void down_arrow(AF* af, BitSet* s, BitSet* r) {
	// TODO: Improve efficiency?
	reset_bitset(r);

	for (SIZE_TYPE i = 0; i < af->size; ++i)
		if (bitset_is_subset(s, af->graph[i]))
			SET_BIT(r, i);
}

// Compute common victims of the total attackers of s
// (arguments commonly attacked by the arguments that attack all elements of s.
// In FCA: down-up-arrow closure operator on the formal context)
// inline void get_common_victims_of_total_attackers(AF* af, BitSet* s, BitSet* r) {
static inline void down_up_arrow(AF* af, BitSet* s, BitSet* r) {
	// First fill r
	// TODO: Improve efficiency?
	set_bitset(r);

	for (SIZE_TYPE i = 0; i < af->size; ++i) {
		if (bitset_is_subset(s, af->graph[i])) {
			bitset_intersection(r, af->graph[i], r);
		}
	}
}

AF* complement_argumentation_framework(AF *af );

AF* transpose_argumentation_framework(AF *af);

AF* create_conflict_framework(AF* af);	// make it undirected

PAF* project_argumentation_framework(AF *af, BitSet* mask);

BitSet* project_back(BitSet* bs, PAF* paf, SIZE_TYPE base_size);

#endif /* AF_AF_H_ */
