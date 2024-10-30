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

#include "../../af/af.h"
#include "complete.h"
#include "../../utils/linked_list.h"

// A complete extension is an admissible extension that contains every argument that it defends.
// I suggest to use the name semi-complete extension for an extension that contains every argument that it defends.
// Semi-complete extensions form a closure system.

// TODO: Optimize
BitSet* closure_semi_complete(AF* attacks, BitSet* s) {
	SIZE_TYPE i;
	BitSet* closure = create_bitset(attacks->size);
	copy_bitset(s, closure);
	bool closure_modified;
	do {
		closure_modified = false;
		for (i = 0; i < attacks->size; i++) {
			if (check_set_defends_arg(attacks, closure, i)) {
				SET_BIT(closure, i);
				closure_modified = true;
			}
		}
	} while (closure_modified);
	return(closure);
}

BitSet* next_conflict_free_intent(AF* not_attacks, AF* attacks, BitSet* previous) {
	BitSet* current = create_bitset(attacks->size);
	copy_bitset(previous, current);
	BitSet* next = create_bitset(attacks->size);

	for (int i = attacks->size - 1; i >= 0; --i) {
		if (TEST_BIT(current, i)) {
			RESET_BIT(current, i);
		} else if (!CHECK_ARG_ATTACKS_ARG(attacks, i, i) &&
				   !CHECK_ARG_ATTACKS_SET(attacks, i, current) &&
				   !check_set_attacks_arg(attacks, current, i)) {
			SET_BIT(current, i);
			down_up_arrow(not_attacks, current, next);

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
				for (SIZE_TYPE j = i + 1; j < attacks->size; ++j) {
					if (TEST_BIT(next, j) && CHECK_ARG_ATTACKS_SET(attacks, j, next)) {
						// we don't check if current attacks j, since this is impossible
						good = false;
						break;
					}
				}
			}
			if (good) {
				return next;
			}
			RESET_BIT(current, i);
		}
	}

	free_bitset(next);
	return 0;
}

ListNode* ee_co_next_closure(AF* af) {
	ListNode* extensions = NULL;
	AF* attacked_by = transpose_argumentation_framework(af);

	free_argumentation_framework(attacked_by);

    return(extensions);
}