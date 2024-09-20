#include <stdlib.h>


#include "../af/af.h"
#include "../utils/linked_list.h"


struct concept {
	BitSet* extent;
	BitSet* intent;
};

typedef struct concept Concept;


int concept_count_bu = 0;


Concept* create_concept_bu(BitSet* extent, BitSet* intent) {
	Concept* c = (Concept*) calloc(1, sizeof(Concept));
	c->extent = extent;
	c->intent = intent;
	++concept_count_bu;
	return(c);
}


void add_to_list_bu(AF* not_attacks, SIZE_TYPE i, ListNode** phead , ListNode** pextensions) {
    ListNode* extensions = *pextensions;

    ListNode* head = *phead;
	ListNode* cur = head;

	while (cur) {
		Concept *c = cur->c;

		if (bitset_is_subset(c->intent, not_attacks->graph[i])) {
			// update c
			SET_BIT(c->extent, i);
		} else {
			// create a new concept
			BitSet *new_intent = create_bitset(not_attacks->size);
			bitset_intersection(c->intent, not_attacks->graph[i], new_intent);

			// "lectic-order test"
			bool is_new_extent_closed = true;
			for (SIZE_TYPE j = 0; j < i; ++j) {
				if (!TEST_BIT(c->extent, j) && bitset_is_subset(new_intent, not_attacks->graph[j])) {
					is_new_extent_closed = false;
					free_bitset(new_intent);
					break;
				}
			}

			if (is_new_extent_closed) {
                BitSet* up = create_bitset(not_attacks->size);
                up_arrow(not_attacks, new_intent, up);
                if (bitset_is_subset(new_intent, up)) { // new_intent is conflict-free
                    if (bitset_is_equal(new_intent, up)) {  // new_intent is stable
                        ListNode* ext = create_node(new_intent);
                        ext->next = extensions;
                        extensions = ext;
                    }
                } else if (bitset_is_subset(up, new_intent)) {  // new_intent is a dominating set
                    BitSet* new_extent = create_bitset(not_attacks->size);
                    copy_bitset(c->extent, new_extent);
                    SET_BIT(new_extent, i);

                    ListNode* new_node = create_node(create_concept_bu(new_extent, new_intent));
                    new_node->next = head;
                    head = new_node;
                }
                free_bitset(up);
            }
        }
		cur = cur->next;
	}
    phead = &head;
    pextensions = &extensions;
}



ListNode* enumerate_stable_extensions_norris_bottom_up(AF* af)
{
	concept_count_bu = 0;
	AF* not_attacks = complement_argumentation_framework(af);

	BitSet* extent = create_bitset(af->size);   // empty set
	BitSet* intent = create_bitset(af->size);
    for (SIZE_TYPE i = 0; i < af->size; ++i) {  // full set
        SET_BIT(intent, i);
    }
	Concept* c = create_concept_bu(extent, intent);
	ListNode* head = create_node(c);

	ListNode* extensions = NULL;
	for (SIZE_TYPE i = 0; i < not_attacks->size; ++i) {
		add_to_list_bu(not_attacks, i, &head, &extensions);
	}

	free_argumentation_framework(not_attacks);

	return extensions;
}


void run_norris_bu(AF* af, FILE* output)
{
    ListNode* head = enumerate_stable_extensions_norris_bottom_up(af);
	ListNode* node = head;

	while (node) {
		print_set(node->c, output, "\n");
		node = node->next;
	}

	while (head) {
		ListNode* next = head->next;
		free_bitset((BitSet*) head->c);
		free(head);
		head = next;
	}
}
