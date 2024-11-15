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

BitSet* dc_pr_cbo(AF* af, SIZE_TYPE a)
{
	if (CHECK_ARG_ATTACKS_ARG(af, a, a)) {
		return 0;
	}

	AF* not_attacks = complement_argumentation_framework(af);

	BitSet* c = create_bitset(af->size);
	//copy_bitset(not_attacks->graph[a], c); ---TODO: This doesn't work as intended!
	set_bitset(c);
	for (SIZE_TYPE i = 0; i < af->size; ++i) {
		if (!CHECK_ARG_ATTACKS_ARG(af, i, a)) {
	        bitset_intersection(c, not_attacks->graph[i], c);
		}
	}
    // c is the closure of {a}

	BitSet* extension = explore_subtree(c, 0, not_attacks, af);

	if (extension != c) {
		free_bitset(c);
	}
	free_argumentation_framework(not_attacks);

    return extension;
}