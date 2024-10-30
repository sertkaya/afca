#include <stdlib.h>


#include "ideal.h"
#include "../../utils/linked_list.h"


SIZE_TYPE update_attacked(AF* af,
                          BitSet* ideal,
                          SIZE_TYPE ideal_count,
                          SIZE_TYPE* ideal_indices, 
                          BitSet* attacked)
{
    reset_bitset(attacked);
    for (SIZE_TYPE i = 0; i < ideal_count;) {
        if (TEST_BIT(ideal, ideal_indices[i])) {
            bitset_union(attacked, af->graph[ideal_indices[i++]], attacked);
        } else {
            // remove i from ideal_indices
            ideal_indices[i] = ideal_indices[--ideal_count];
        }
    }
    return ideal_count;
}


SIZE_TYPE update_attackers(AF* af,
                           BitSet* attackers,
                           BitSet* ideal,
                           SIZE_TYPE attackers_count,
                           SIZE_TYPE* attacker_indices)
{
    for (SIZE_TYPE i = 0; i < attackers_count;) {
        if (is_bitset_intersection_empty(ideal, af->graph[attacker_indices[i]])) {
            // remove attacker i
            RESET_BIT(attackers, attacker_indices[i]);
            attacker_indices[i] = attacker_indices[--attackers_count];
        } else {
            ++i;
        }
    }
    return attackers_count;
}


BitSet* se_id(AF* af, ListNode* (*preferred_extensions)(AF* af))
{
    ListNode* preferred = preferred_extensions(af);
    BitSet* ideal = (BitSet*) preferred->c;
    ListNode* next = preferred->next;
    free_list_node(preferred);
    while (next) {
        preferred = next;
        next = preferred->next;
        BitSet* p = (BitSet*) preferred->c;
        bitset_intersection(ideal, p, ideal);
        free_bitset(p);
        free_list_node(preferred);
    }

    SIZE_TYPE ideal_count = count_bits(ideal);
    if (0 == ideal_count) {
        return ideal;
    }

    SIZE_TYPE* ideal_indices = (SIZE_TYPE*) malloc(ideal_count * sizeof(SIZE_TYPE));
    SIZE_TYPE j = 0;
    BitSet* attacked = create_bitset(af->size);
    for (SIZE_TYPE i = 0; i < af->size; ++i) {
        if (TEST_BIT(ideal, i)) {
            ideal_indices[j++] = i;
            bitset_union(attacked, af->graph[i], attacked);
        }
    }


    SIZE_TYPE* attacker_indices = (SIZE_TYPE*) malloc((af->size - ideal_count) * sizeof(SIZE_TYPE));
    SIZE_TYPE attackers_count = 0;
    BitSet* attackers = create_bitset(af->size);
    for (SIZE_TYPE i = 0; i < af->size; ++i) {
        if (!is_bitset_intersection_empty(ideal, af->graph[i])) {
            attacker_indices[attackers_count++] = i;
            SET_BIT(attackers, i);
        }
    }

    while (!bitset_is_subset(attackers, attacked)) {
        for (SIZE_TYPE i = 0; i < attackers_count; ++i) {
            if (!TEST_BIT(attacked, attacker_indices[i])) {
                bitset_set_minus(ideal, af->graph[attacker_indices[i]], ideal);
                ideal_count = update_attacked(af, ideal, ideal_count, ideal_indices, attacked);
                attackers_count = update_attackers(af, attackers, ideal, attackers_count, attacker_indices);
                break;
            }
        }
    }

    free(attacker_indices);
    free(ideal_indices);

    free_bitset(attackers);
    free_bitset(attacked);

    return ideal;
}