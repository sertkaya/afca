#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <inttypes.h>

#include "../fca/context.h"
#include "../fca/concept.h"
#include "../bitset/bitset.h"
#include "../utils/linked_list.h"

Concept* create_concept(BitSet* extent,
		BitSet* intent,
		BitSet* not_attacked,
		BitSet* conflict_free) {
	Concept* c = (Concept*) calloc(1, sizeof(Concept));
	assert(c != NULL);
	c->extent = extent;
	c->intent = intent;
	c->not_attacked = not_attacked;
	c->conflict_free = conflict_free;
	return(c);
}


int add(Context* not_attacks, int i, ListNode *head , BitSet** argument_extents, FILE *outfile) {

	int stable_extension_count = 0;

	ListNode *prev = NULL;
	ListNode *cur = head;

	// new concepts
	ListNode *new_head = NULL;

	while (cur) {
		Concept *c = cur->c;
		if (TEST_BIT(c->conflict_free, i)) {
			// c->intent U {i} is a conflict-free superset of c->intent

			if (bitset_is_subset(c->extent, argument_extents[i])) {
				// update c
				SET_BIT(c->intent, i);
				bitset_intersection(c->not_attacked, not_attacks->a[i], c->not_attacked);

				char remove_cur = 0;
				if (bitset_is_equal(c->intent, c->not_attacked)) {
					// c->intent is a stable extension
					++stable_extension_count;
					print_bitset(c->intent, outfile);
					fprintf(outfile, "\n");
					remove_cur = 1;
				} else {
					bitset_intersection(c->conflict_free, not_attacks->a[i], c->conflict_free);
					if (bitset_is_equal(c->conflict_free, c->intent)) {
						remove_cur = 1;
					}
				}

				if (remove_cur) {
					cur = cur->next;
					// remove cur from the concept list
					if (prev) {
						free(prev->next);
						prev->next = cur;
					} else {
						free(head);
						head = cur;
					}
					continue;
				}
			} else {
				// create a new concept
				BitSet *new_extent = create_bitset(not_attacks->size);
				bitset_intersection(c->extent, argument_extents[i], new_extent);

				// "lectic-order test"
				char is_new_intent_closed = 0;
				for (int j = 0; j < i; ++j) {
					if (!(TEST_BIT((c->intent),j)) && (bitset_is_subset(new_extent, argument_extents[j]))) {
						is_new_intent_closed = 0;
						free_bitset(new_extent);
						break;
					}
				}

				if (is_new_intent_closed) {
					BitSet* new_intent = create_bitset(not_attacks->size);
					copy_bitset(c->intent, new_intent);
					SET_BIT(new_intent, i);

					BitSet* new_not_attacked = create_bitset(not_attacks->size);
					bitset_intersection(c->not_attacked, not_attacks->a[i], new_not_attacked);

					if (bitset_is_equal(new_intent, new_not_attacked)) {
						// new_intent is a stable extension
						++stable_extension_count;
						print_bitset(new_intent, outfile);
						fprintf(outfile, "\n");
						// TODO: deallocate memory for new_*
					} else {
						BitSet* new_conflict_free = create_bitset(not_attacks->size);
						bitset_intersection(new_extent, new_not_attacked, new_conflict_free);

						if (!bitset_is_equal(new_conflict_free, new_intent)) {
							Concept *new_concept = create_concept(new_extent,
									new_intent,
									new_not_attacked,
									new_conflict_free);
							ListNode *new_node = create_node(new_concept);
							if (new_head) {
								new_node->next = new_head;
							}
							new_head = new_node;
						}
						else {
							// deallocate memory for new_*
							free_bitset(new_intent);
							free_bitset(new_extent);
							free_bitset(new_not_attacked);
							free_bitset(new_conflict_free);
						}
					} // closed but not stable
				} // closed
			} // new concept
		} // i is conflict free with cur concept
		prev = cur;
		cur = cur->next;
	}
	if (prev) {
		prev->next = new_head;
	}

	return stable_extension_count;
}

void incremental_stable_extensions(Context* attacks, FILE *outfile) {

	Context* not_attacks = negate_context(attacks);

	BitSet* argument_extents[attacks->size];
	for (int i = 0; i < not_attacks->size; ++i) {
		argument_extents[i] = create_bitset(not_attacks->size);
		for (int j = 0; j < not_attacks->size; ++j) {
			if (TEST_BIT(not_attacks->a[j], i)) {
				SET_BIT(argument_extents[i], j);
			}
		}
	}

	BitSet* intent = create_bitset(attacks->size);  // empty set
	BitSet* extent = create_bitset(attacks->size);
	negate_bitset(intent, extent);                  // full set
	BitSet* not_attacked = create_bitset(attacks->size);
	copy_bitset(extent, not_attacked);
	BitSet* conflict_free = create_bitset(attacks->size);
	copy_bitset(extent, conflict_free);
	Concept* c = create_concept(extent, intent, not_attacked, conflict_free);
	ListNode* head = create_node(c);

	int stable_extension_count = 0;

	for (int i = 0; i < not_attacks->size; ++i) {
		stable_extension_count += add(not_attacks, i, head, argument_extents, outfile);
	}

	printf("Number of stable extensions: %d\n", stable_extension_count);
}
