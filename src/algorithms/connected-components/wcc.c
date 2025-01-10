#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <inttypes.h>


#include "../../af/af.h"
#include "../bitset/bitset.h"
#include "../utils/linked_list.h"


void undirected_dfs(AF* af, int i, bool* visited, BitSet* component) {
    SET_BIT(component, i);
    visited[i] = true;
    for (unsigned short j = 1; j < af->size; ++j) {
        if (!visited[j] && (TEST_BIT(af->graph[i], j) || TEST_BIT(af->graph[j], i))) {
            undirected_dfs(af, j, visited, component);
        }
    }
}


ListNode* wcc(AF* af, unsigned short* n) {
    // components are stored in ListNodes
    // each component is stored as a BitSet mask
    bool visited[af->size];
    for (unsigned short i = 0; i < af->size; ++i) {
        visited[i] = false;
    }
    
    ListNode* first_component = NULL;
    *n = 0;
    
    for (unsigned short i = 0; i < af->size; ++i) {
        if (!visited[i]) {
            BitSet* mask = create_bitset(af->size);
            undirected_dfs(af, i, visited, mask);
            ListNode* new_component = create_list_node(mask);
            new_component->next = first_component;
            // TODO: new_component->c is not set! Causes segmentation fault later.
            first_component = new_component;
            (*n)++;
        }
    }
    return first_component;
}


void free_components(ListNode* first_component) {
    ListNode* component = first_component;
    while (component) {
        ListNode* next = component->next;
        free(component->c);
        free_list_node(component);
        component = next;
    }
}


void free_extension_lists(ListNode** extension_lists, unsigned short begin, unsigned short end){
    for (unsigned short i = begin; i < end; ++i) {
        ListNode* component_extension = extension_lists[i];
        while (component_extension) {
            ListNode* next = component_extension->next;
            free_bitset((BitSet*) component_extension->c);
            free(component_extension);
            component_extension = next;
        }
    }
}


void free_projections(PAF** projections, unsigned short n) {
    for (unsigned short i; i < n; ++i) {
        free_projected_argumentation_framework(projections[i]);
    }
}


void restore_indices(ListNode** extension_lists, PAF** projections, unsigned short n, unsigned short base_size) {
    for (int i = 0; i < n; ++i) {
        PAF* paf = projections[i];
        ListNode* node = extension_lists[i];
        while (node) {
            BitSet* projected_extension = node->c;
            node->c = project_back(node->c, paf);
            free_bitset(projected_extension);
            node = node->next;
        }
    }
}


ListNode* wcc_stable_extensions(AF* af, ListNode* (*stable_extensions)(AF* af)) {
    unsigned short n;
    ListNode* first_component = wcc(af, &n);
    printf("Weakly connected components: %d\n", n);
    ListNode* extension_lists[n];
    PAF* projections[n];

    // compute "local" stable extensions in each component
    ListNode* component = first_component;
    for (unsigned short i = 0; i < n; ++i) {
        projections[i] = project_argumentation_framework(af, component->c);
        printf("Number of arguments in component %d: %d\n", i + 1, projections[i]->af->size);
        extension_lists[i] = stable_extensions(projections[i]->af);
        if (!extension_lists[i]) {
            // there are no "global" stable extensions either
            free_projections(projections, i + 1);
            free_components(first_component);
            free_extension_lists(extension_lists, 0, i);
            return NULL;
        }

        component = component->next;
    }

    free_components(first_component);

    restore_indices(extension_lists, projections, n, af->size);
 
    // form the unions of stable extensions from different components
    // and store the results in extension_lists[0]
    ListNode* res = extension_lists[0];
    for (unsigned short i = 1; i < n; ++i) {
        // printf("Component %d\n", i);
        ListNode* cur = res;
        while (cur) {
            // printf("Before: ");
            // print_set(cur->c, stdout, "\n");
            BitSet* curset = (BitSet*) cur->c;
            ListNode* next = cur->next;

            ListNode* component_extension = extension_lists[i];
            // form unions of cur and extensions from the ith component
            do {
                // printf("Other: ");
                // print_set(component_extension->c, stdout, "\n");
                cur->c = create_bitset(curset->size);
                bitset_union(curset, component_extension->c, cur->c);
                // printf("Union: ");
                // print_set(cur->c, stdout, "\n");
                component_extension = component_extension->next;
                cur->next = component_extension ? create_list_node(NULL) : next;
                cur = cur->next;
            } while (component_extension);

            free_bitset((BitSet*) curset);
        }
    }

    free_projections(projections, n);
    free_extension_lists(extension_lists, 1, n);

    return res;
}