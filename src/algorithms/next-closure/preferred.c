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
#include "../../utils/linked_list.h"


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
				free_bitset(current);
				return next;
			}
			RESET_BIT(current, i);
		}
	}

	free_bitset(current);
	free_bitset(next);
	return 0;
}

/// @brief Adds c to the list unless c is a subset of another node; removes subsets of c
/// @param head is the head of the list
/// @param c is the set to be added
/// @return the new head of the list
ListNode* add_candidate(ListNode* head, BitSet* c)
{
    ListNode* prev = 0;
    ListNode* cur = head;
    while (cur) {
        if (bitset_is_subset(c, cur->c)) {
            return head;
        }
        if (bitset_is_subset(cur->c, c)) {
            ListNode* old = cur;
            cur = cur->next;
            if (prev) {
                prev->next = cur;
            } else {
                head = cur;
            }
            free_bitset(old->c);
            free_list_node(old);
        } else {
            prev = cur;
            cur = cur->next;
        }
    }

    return(insert_list_node(c, head));
}


bool is_conflict_free_set_admissible(BitSet* s, AF* not_attacks, BitSet* up, BitSet* down)
{
	up_arrow(not_attacks, s, up);
    down_arrow(not_attacks, s, down);
    return bitset_is_subset(up, down);
}


ListNode* ee_pr_next_closure(AF* af)
{
    BitSet* up = create_bitset(af->size);
    BitSet* down = create_bitset(af->size);

	AF* not_attacks = complement_argumentation_framework(af);

	BitSet* c = create_bitset(af->size);
    copy_bitset(af->graph[0], c);
	for (SIZE_TYPE i = 1; i < af->size && !bitset_is_emptyset(c); ++i) {
        bitset_intersection(c, af->graph[i], c);
	}
    // c is the closure of the empty attribute set,
    // i.e., c is the set of arguments not attacked by any argument

    ListNode* first_candidate = 0;
    BitSet* new_intent = 0;
	BitSet* prev_intent = c;

    while ((new_intent = next_conflict_free_intent(not_attacks, af, prev_intent))) {
		if (prev_intent != c) {
			free_bitset(prev_intent);
		}
        if (!c || bitset_is_subset(c, new_intent)) {  // still on the same computational branch
            if (is_conflict_free_set_admissible(new_intent, not_attacks, up, down)) {
                if (c) {
					free_bitset(c); // c is not maximal admissible
				}
                c = new_intent;
            }
        } else {    // c is maximal admissible on its computational branch
            first_candidate = add_candidate(first_candidate, c);
			c = is_conflict_free_set_admissible(new_intent, not_attacks, up, down) ? new_intent : 0;
        }
		prev_intent = new_intent;
    }
	if (c) {
		first_candidate = add_candidate(first_candidate, c);
	}

	free_argumentation_framework(not_attacks);
	free_bitset(down);
	free_bitset(up);

    return(first_candidate);
}