#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <inttypes.h>

#include "../fca/context.h"
// #include "../fca/concept.h"
#include "../bitset/bitset.h"
#include "../utils/linked_list.h"

Concept* create_concept(BitSet* extent,
		BitSet* intent,
		BitSet* not_attacked) {
	Concept* c = (Concept*) calloc(1, sizeof(Concept));
	assert(c != NULL);
	c->extent = extent;
	c->intent = intent;
	c->not_attacked = not_attacked;
	return(c);
}

void free_concept(Concept *c) {
	free_bitset(c->extent);
	free_bitset(c->intent);
	free_bitset(c->not_attacked);
	free(c);
}

int add(Context* not_attacks, int i, ListNode **phead , BitSet** argument_extents, FILE *outfile) {

	int stable_extension_count = 0;

    ListNode *head = *phead;
	ListNode *prev = NULL;
	ListNode *cur = head;

	// new concepts
	ListNode *new_head = NULL;

	BitSet *conflict_free = create_bitset(not_attacks->size);

	while (cur) {
		Concept *c = cur->c;
		bitset_intersection(c->extent, c->not_attacked, conflict_free);
		if (TEST_BIT(conflict_free, i)) {
			// c->intent U {i} is a conflict-free superset of c->intent
			// print_bitset(c->intent, stdout);
			// printf("\n");

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
					bitset_intersection(conflict_free, not_attacks->a[i], conflict_free);
					if (bitset_is_equal(conflict_free, c->intent)) {
						remove_cur = 1;
					}
				}

				if (remove_cur) {
					cur = cur->next;
					// remove cur from the concept list
					if (prev) {
						free_concept(prev->next->c);
						free_node(prev->next);
						prev->next = cur;
					} else {
						free_concept(head->c);
						free_node(head);
						head = cur;
					}
					continue;
				}
			} else {
				// create a new concept
				BitSet *new_extent = create_bitset(not_attacks->size);
				bitset_intersection(c->extent, argument_extents[i], new_extent);

				// "lectic-order test"
				char is_new_intent_closed = 1;
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
						free_bitset(new_intent);
						free_bitset(new_extent);
						free_bitset(new_not_attacked);
					} else {
						BitSet* new_conflict_free = create_bitset(not_attacks->size);
						bitset_intersection(new_extent, new_not_attacked, new_conflict_free);

						if (!bitset_is_equal(new_conflict_free, new_intent)) {
							Concept *new_concept = create_concept(new_extent,
									new_intent,
									new_not_attacked);
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
    phead = &head;

	return stable_extension_count;
}

void incremental_stable_extensions_norris(Context* attacks, FILE *outfile) {

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
    for (int i = 0; i < attacks->size; ++i) {       // full set
        SET_BIT(extent, i);
    }
	BitSet* not_attacked = create_bitset(attacks->size);
	copy_bitset(extent, not_attacked);
	Concept* c = create_concept(extent, intent, not_attacked);
	ListNode* head = create_node(c);

	int stable_extension_count = 0;

	for (int i = 0; i < not_attacks->size; ++i) {
		stable_extension_count += add(not_attacks, i, &head, argument_extents, outfile);
	}

	printf("Number of stable extensions: %d\n", stable_extension_count);

	free_concept(c);
	free_node(head);
	for (int i = 0; i < not_attacks->size; ++i)
		free_bitset(argument_extents[i]);
	free_context(attacks);
	free_context(not_attacks);

}

int add_one(Context* not_attacks, int i, ListNode **phead , BitSet** argument_extents, FILE *outfile) {

    ListNode *head = *phead;
	ListNode *prev = NULL;
	ListNode *cur = head;

	// new concepts
	ListNode *new_head = NULL;

	BitSet *conflict_free = create_bitset(not_attacks->size);

	while (cur) {
		Concept *c = cur->c;
		bitset_intersection(c->extent, c->not_attacked, conflict_free);
		if (TEST_BIT(conflict_free, i)) {
			// c->intent U {i} is a conflict-free superset of c->intent
			// print_bitset(c->intent, stdout);
			// printf("\n");

			if (bitset_is_subset(c->extent, argument_extents[i])) {
				// update c
				SET_BIT(c->intent, i);
				bitset_intersection(c->not_attacked, not_attacks->a[i], c->not_attacked);

				char remove_cur = 0;
				if (bitset_is_equal(c->intent, c->not_attacked)) {
					// c->intent is a stable extension
					print_bitset(c->intent, outfile);
					fprintf(outfile, "\n");
					remove_cur = 1;
					return(1);
				} else {
					bitset_intersection(conflict_free, not_attacks->a[i], conflict_free);
					if (bitset_is_equal(conflict_free, c->intent)) {
						remove_cur = 1;
					}
				}

				if (remove_cur) {
					cur = cur->next;
					// remove cur from the concept list
					if (prev) {
						free_concept(prev->next->c);
						free_node(prev->next);
						prev->next = cur;
					} else {
						free_concept(head->c);
						free_node(head);
						head = cur;
					}
					continue;
				}
			} else {
				// create a new concept
				BitSet *new_extent = create_bitset(not_attacks->size);
				bitset_intersection(c->extent, argument_extents[i], new_extent);

				// "lectic-order test"
				char is_new_intent_closed = 1;
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
						print_bitset(new_intent, outfile);
						fprintf(outfile, "\n");
						// TODO: deallocate memory for new_*
						free_bitset(new_intent);
						free_bitset(new_extent);
						free_bitset(new_not_attacked);
						return(1);
					} else {
						BitSet* new_conflict_free = create_bitset(not_attacks->size);
						bitset_intersection(new_extent, new_not_attacked, new_conflict_free);

						if (!bitset_is_equal(new_conflict_free, new_intent)) {
							Concept *new_concept = create_concept(new_extent,
									new_intent,
									new_not_attacked);
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
    phead = &head;

	return(0);
}

void one_stable_extension_norris(Context* attacks, FILE *outfile) {

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
    for (int i = 0; i < attacks->size; ++i) {       // full set
        SET_BIT(extent, i);
    }
	BitSet* not_attacked = create_bitset(attacks->size);
	copy_bitset(extent, not_attacked);
	BitSet* conflict_free = create_bitset(attacks->size);
	copy_bitset(extent, conflict_free);
	Concept* c = create_concept(extent, intent, not_attacked);
	ListNode* head = create_node(c);

	for (int i = 0; i < not_attacks->size; ++i) {
		if (add(not_attacks, i, &head, argument_extents, outfile))
			break;
	}

	free_concept(c);
	free_node(head);
	for (int i = 0; i < not_attacks->size; ++i)
		free_bitset(argument_extents[i]);
	free_context(attacks);
	free_context(not_attacks);

}
