#include "mis.h"


ListNode* extend(SIZE_TYPE i, 
                 BitSet* s, 
                 BitSet* conflicting, 
                 BitSet* dominated, // consists of s and its victims
                 AF* af, 
                 AF* conflicts, 
                 ListNode* extensions)
{
    while (i < af->size && !TEST_BIT(conflicting, i)) {
        if (!TEST_BIT(conflicts->graph[i], i)) {
            SET_BIT(s, i);
            SET_BIT(dominated, i);
            bitset_union(conflicting, conflicts->graph[i], conflicting);
            bitset_union(dominated, af->graph[i], dominated);
        }
        ++i;
    }

    if (i == af->size) {
        if (bitset_is_fullset(dominated)) {
            extensions = insert_list_node(s, extensions);
        } else {
            free_bitset(s);
        }
        free_bitset(conflicting);
        free_bitset(dominated);
        return extensions;
    }

    BitSet* iset = create_bitset(af->size);
    bitset_set_minus(s, conflicts->graph[i], iset);

    bool canonical = !TEST_BIT(conflicts->graph[i], i);
    if (canonical) {
        BitSet* canonical_parent = create_bitset(af->size);
        copy_bitset(iset, canonical_parent);
        SET_BIT(iset, i);
        for (SIZE_TYPE j = 0; j < i; ++j) {
            if(!TEST_BIT(iset, j)) {
                if (is_bitset_intersection_empty(iset, conflicts->graph[j])) {
                    // iset is not maximal conflict-free
                    canonical = false;
                    break;
                }
                if (is_bitset_intersection_empty(canonical_parent, conflicts->graph[j])) {
                    if (!TEST_BIT(s, j)) {
                        // iset can be generated as a child of canonical_parent U {j} or its superset
                        canonical = false;
                        break;
                    }
                    SET_BIT(canonical_parent, j);
                }
            }
        }
        free_bitset(canonical_parent);
    }

    extensions = extend(i + 1, s, conflicting, dominated, af, conflicts, extensions);

    if (canonical) {
        BitSet* idominated = create_bitset(af->size);
        get_victims(af, iset, idominated);

        BitSet* iconflicting = create_bitset(af->size);
        get_true_attackers(af, iset, iconflicting);
        bitset_union(iconflicting, idominated, iconflicting);

        bitset_union(idominated, iset, idominated);

        extensions = extend(i + 1, iset, iconflicting, idominated, af, conflicts, extensions);
    } else {
        free_bitset(iset);
    }

    return extensions;
}

ListNode* ee_st_maximal_independent_sets(AF *af)
{
    AF* conflicts = create_conflict_framework(af);

    BitSet* s = create_bitset(af->size);
    BitSet* conflicting = create_bitset(af->size);
    BitSet* dominated = create_bitset(af->size);

    ListNode* extensions = extend(0, s, conflicting, dominated, af, conflicts, 0);

    free_argumentation_framework(conflicts);

    return extensions;
}