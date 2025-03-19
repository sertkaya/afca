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

#include "preferred.h"

BitSet* explore_subtree(BitSet* current, SIZE_TYPE m, AF* not_attacks, AF* af)
{
	BitSet* extension = 0;
	BitSet* up = create_bitset(af->size);
	up_arrow(not_attacks, current, up);
	bool conflict_free = bitset_is_subset(current, up);

	if (conflict_free) {
		BitSet* next = create_bitset(af->size);

		for (SIZE_TYPE i = m; i < af->size; ++i) {
			if (!TEST_BIT(current, i) && 
				!CHECK_ARG_ATTACKS_ARG(af, i, i) &&
			  	!CHECK_ARG_ATTACKS_SET(af, i, current) &&
			  	!check_set_attacks_arg(af, current, i)) {

				SET_BIT(current, i);
				down_up_arrow(not_attacks, current, next);
				RESET_BIT(current, i);

				bool good = true;
				// is next canonical?
				for (SIZE_TYPE j = 0; j < i; ++j) {
					if (TEST_BIT(next, j) && !TEST_BIT(current, j)) {
						good = false;
						break;
					}
				}

				if (good) {
					// is next conflict-free?
					for (SIZE_TYPE j = i + 1; j < af->size; ++j) {
						if (TEST_BIT(next, j) && CHECK_ARG_ATTACKS_SET(af, j, next)) {
							// we don't check if current attacks j, since this is impossible
							good = false;
							break;
						}
					}
				}

				if (good) {
					extension = explore_subtree(next, i + 1, not_attacks, af);
					if (extension) {
						break;
					}
				}
			}
		} // end for

		if (!extension) {
			down_arrow(not_attacks, current, next);
			if (bitset_is_subset(up, next)) {
				extension = current;
			}
		}

		if (extension != next) {
			free_bitset(next);
		}
	}

	free_bitset(up);
	
	return extension;
}


int select_best_attacker(BitSet* current, BitSet* processed, AF* not_attacks, AF* attacked_by)
{
	// Return an unattacked attacker of current
	// that has the smallest number of attackers outside processed.
	// If current has no such attackers, i.e., it is self-defending,
	// return -1

	BitSet* unattacked = create_bitset(current->size);
	up_arrow(not_attacks, current, unattacked);

	BitSet* nonattacking = create_bitset(current->size);
	down_arrow(not_attacks, current, nonattacking);

	BitSet* attackers = create_bitset(current->size);

	int best = -1;
	SIZE_TYPE min_attackers = not_attacks->size + 1;
	for (SIZE_TYPE i = 0; i < current->size; ++i)
	{
		if (TEST_BIT(unattacked, i) && !TEST_BIT(nonattacking, i)) {
			bitset_set_minus(attacked_by->graph[i], processed, attackers);
			SIZE_TYPE k = count_bits(attackers);
			if (k < min_attackers) {
				min_attackers = k;
				best = i;
				if (min_attackers == 0) {
					break;
				}
			}
		}
	}
	free_bitset(unattacked);
	free_bitset(nonattacking);
	free_bitset(attackers);
	return best;
}

BitSet* explore_subtree_smart(BitSet* current, BitSet* processed, AF* not_attacks, AF* af, AF* attacked_by)
{
	// Return maximal admissible superset of current
	// or NULL if no admissible superset exists.

	// current must be conflict-free
	// processed must contain (among others) current,
	// all self-attacking arguments, and
	// arguments in conflict with current

	BitSet* extension = NULL;
	BitSet* next = create_bitset(af->size);
	BitSet* next_processed = create_bitset(af->size);

	// select an unattacked attacker with the smallest number of unprocessed attackers
	int attacker = select_best_attacker(current, processed, not_attacks, attacked_by);
	for (SIZE_TYPE i = 0; i < af->size; ++i) {

		if (TEST_BIT(processed, i) ||
			// if there is attacker, i must attack it
			(attacker >= 0 && !CHECK_ARG_ATTACKS_ARG(af, i, attacker))) {
			continue;
		}

		SET_BIT(current, i);
		down_up_arrow(not_attacks, current, next);
		RESET_BIT(current, i);

		copy_bitset(processed, next_processed);

		bool good = true;
		for (SIZE_TYPE j = 0; j < af->size; ++j) {
			if (TEST_BIT(next, j) && !TEST_BIT(current, j)) {
				if (TEST_BIT(processed, j) ||	// non-canonical?
					CHECK_ARG_ATTACKS_SET(af, j, next)) { // conflicts?
					good = false;
					break;
				} else {
					bitset_union(next_processed, af->graph[j], next_processed);
					SET_BIT(next_processed, j);
				}
			}
		}

		if (good) {
			// arguments attacking neither current nor i don't attack next
			// so new attackers of next (compared to current) are among attackers of i
			bitset_union(next_processed, attacked_by->graph[i], next_processed);
			extension = explore_subtree_smart(next, next_processed, not_attacks, af, attacked_by);
			if (extension) {
				break;
			}
		}

		SET_BIT(processed, i);
	}

	if (!extension && attacker < 0) {
		// current is admissible, since it has no unattacked attackers
		extension = current;
	}

	if (extension != next) {
		free_bitset(next);
	}

	free_bitset(next_processed);

	return extension;
}

BitSet* dc_pr_cbo(AF* af, SIZE_TYPE a)
{
	if (a < af->size && CHECK_ARG_ATTACKS_ARG(af, a, a)) {
		return 0;
	}

	AF* not_attacks = complement_argumentation_framework(af);

	BitSet* c = create_bitset(af->size);
	//copy_bitset(not_attacks->graph[a], c); ---TODO: This doesn't work as intended!
	//										 ---TODO: It should now (after changes in complement_bitset).
	set_bitset(c);
	for (SIZE_TYPE i = 0; i < af->size; ++i) {
		if (a == af->size || !CHECK_ARG_ATTACKS_ARG(af, i, a)) {
	        bitset_intersection(c, not_attacks->graph[i], c);
		}
	}
    // c is the closure of {a} if a >= 0 or of {} otherwise

	BitSet* extension = explore_subtree(c, 0, not_attacks, af);

	if (extension != c) {
		free_bitset(c);
	}
	free_argumentation_framework(not_attacks);

    return extension;
}


BitSet* dc_pr_cbo_smart(AF* af, SIZE_TYPE a)
{
	// printf("smart\n");
	if (a < af->size && CHECK_ARG_ATTACKS_ARG(af, a, a)) {
		return NULL;
	}

	AF* not_attacks = complement_argumentation_framework(af);
	BitSet* processed = create_bitset(af->size);

	BitSet* c = create_bitset(af->size);
	set_bitset(c);
	for (SIZE_TYPE i = 0; i < af->size; ++i) {
		if (a == af->size || !CHECK_ARG_ATTACKS_ARG(af, i, a)) {
			bitset_intersection(c, not_attacks->graph[i], c);
			if (CHECK_ARG_ATTACKS_ARG(af, i, i)) {
				SET_BIT(processed, i);	// self-attacking arguments
			}
		} else {
			SET_BIT(processed, i);	// attackers of c
		}
	}
	// c is the closure of {a} if a < af->size or of {} otherwise
	// (recall that preferred extensions are closed)

	bitset_union(c, processed, processed);	// c is contained in processed
	for (SIZE_TYPE i = 0; i < af->size; ++i) {
		if (TEST_BIT(c, i)) {
			// add arguments attacked by c
			bitset_union(processed, af->graph[i], processed);
		}
	}

	AF* attacked_by = transpose_argumentation_framework(af);
	BitSet* extension = explore_subtree_smart(c, processed, not_attacks, af, attacked_by);

	if (extension != c) {
		free_bitset(c);
	}

	free_bitset(processed);
	free_argumentation_framework(not_attacks);
	free_argumentation_framework(attacked_by);

	return extension;
}


BitSet* ds_pr_cbo(AF* af, SIZE_TYPE a)
{
	BitSet* extension = se_pr_cbo(af);
	if (!TEST_BIT(extension, a)) {
		return extension;
	}
	free_bitset(extension);
	extension = 0;

	// TODO: This is based on the assumption that every preferred extension
	// without a must attack a. Check if this is true.
	for (SIZE_TYPE i = 0; i < af->size; ++i) {
		if (CHECK_ARG_ATTACKS_ARG(af, i, a)) {
			// TODO: Calls for different i may repeat some work. Optimize.
			extension = dc_pr_cbo(af, i);
			if (extension) {
				break;
			}
		}
	}

	return extension;
}


BitSet* se_pr_cbo(AF* af)
{
	return dc_pr_cbo_smart(af, af->size);
	// return dc_pr_cbo(af, af->size);
}
