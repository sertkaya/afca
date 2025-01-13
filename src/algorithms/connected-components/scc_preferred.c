#include "../../af/af.h"
#include "../utils/linked_list.h"
#include "../utils/map.h"
#include "scc.h"

#ifndef MAP_SIZE
#define MAP_SIZE 	101


void global_dfs(AF* af, SIZE_TYPE i, BitSet* visited) {
    SET_BIT(visited, i);
    // assume that vertex 0 is already visited
    for (SIZE_TYPE j = 1; j < af->size; ++j) { 
        if (!TEST_BIT(visited, j) && TEST_BIT(af->graph[i], j)) {
            global_dfs(af, j, visited);
        }
    }
}


void global_backward_dfs(AF* af, SIZE_TYPE i, BitSet* visited) {
    SET_BIT(visited, i);
    for (SIZE_TYPE j = 0; j < af->size; ++j) {
        if (!TEST_BIT(visited, j) && TEST_BIT(af->graph[j], i)) {
            global_backward_dfs(af, j, visited);
        }
    }
}


// find a source component in af
// component must be empty
void find_global_source_component(AF* af, BitSet* component) {
    // find a vertex in a source component
    SIZE_TYPE source;
    for (SIZE_TYPE i = 0; i < af->size; ++i) {
        if (!TEST_BIT(component, i)) {
            source = i;
            global_dfs(af, i, component);
        }
    }

    // extract the component containing source
    // printf("Source vertex: %d\n", source);
    reset_bitset(component);
    global_backward_dfs(af, source, component);
}


ListNode* get_component_preferred_extensions(PAF* paf,
                                             BitSet* component,
                                             ListNode* (*preferred_extensions)(AF* af)) {
    PAF* projection = project_paf_with_loops(paf, component, NULL);
    ListNode* component_extension = preferred_extensions(projection->af);
    restore_base_indices(component_extension, projection);
    free_projected_argumentation_framework(projection);
    return component_extension;
}

// component_extension has base indices
PAF* extract_preferred_residual_framework(PAF* paf, 
                                          BitSet* source_component, 
                                          BitSet* component_extension) {
    BitSet* remainder = create_bitset(paf->af->size);
    set_bitset(remainder);
    BitSet* undefended = create_bitset(paf->af->size);

    BitSet* to_remove = create_bitset(paf->af->size);
    for (SIZE_TYPE i = 0; i < paf->af->size; ++i) {
        if (TEST_BIT(component_extension, paf->base_mapping[i])) {
            // delete arguments in component_extension 
            RESET_BIT(remainder, i);
            // printf("remove %d\n", paf->base_mapping[i] + 1);
            // delete arguments attacked by component_extension 
            bitset_set_minus(remainder, paf->af->graph[i], remainder);
            // printf("remove ");
            // print_set(paf->af->graph[i], stdout, "\n");
        }
    }
    for (SIZE_TYPE i = 0; i < paf->af->size; ++i) {
        if (TEST_BIT(source_component, i) && TEST_BIT(remainder, i)) {
            // victims of a source_component argument that is not attacked by component_extension
            // cannot be defended
            bitset_union(undefended, paf->af->graph[i], undefended);
            // delete arguments in source_component
            RESET_BIT(remainder, i);
            // printf("remove %d\n", paf->base_mapping[i] + 1);
        }
	}
    PAF* residual = bitset_is_emptyset(remainder) ?
                    NULL : 
                    project_paf_with_loops(paf, remainder, undefended); 
    free_bitset(remainder);
    free_bitset(undefended);
	return residual;
}


void free_extension_node(ListNode* node){
    free_bitset((BitSet*) node->c);
    free_list_node(node);
}


ListNode* advance_and_free_extension(ListNode* node) {
    ListNode* next = node->next;
    free_extension_node(node);
    return next;
}


// compute preferred extensions in paf
// arguments specifies which arguments of the original framework are present in paf
// added_loops specifies loops added to paf in comparison with the original framework
ListNode* compute_preferred_extensions(PAF* paf,
                                       // BitSet* arguments,
                                       // BitSet* added_loops,
                                       ListNode* (*preferred_extensions)(AF* af),
                                       Map* subextensions) {
    // printf("Arguments: %d\n", paf->af->size);
    BitSet* component = create_bitset(paf->af->size);
    find_global_source_component(paf->af, component);
    // printf("Component size: %d\n", count_bits(component));

    // TODO: Use hashing
    PAF* component_paf = project_paf_with_loops(paf, component, NULL);
    // printf("PAF\n");
    ListNode* component_extension = preferred_extensions(component_paf->af);
    restore_base_indices(component_extension, component_paf);

    if (count_bits(component) == paf->af->size) {
        free_bitset(component);
        free_projected_argumentation_framework(component_paf);
        return component_extension;
    }

    ListNode* head = create_list_node(NULL);
    ListNode* last_node = head;

    while (component_extension) {
        // printf("Arguments in component extension: %d\n", count_bits(component_extension->c));
        PAF* residual_framework = extract_preferred_residual_framework(paf, 
                                                                       component,
                                                                       component_extension->c);

        if (residual_framework) {
            // printf("Residual arguments: %d\n", residual_framework->af->size);
            ListNode* residual_extension = compute_preferred_extensions(residual_framework, preferred_extensions, subextensions);
            // restore_base_indices(residual_extension, paf);
            // printf("EXT: ");
            // print_set(residual_extension->c, stdout, "\n");
            while (residual_extension) {
                last_node->next = create_list_node(create_bitset(paf->base_size));
                last_node = last_node->next;
                bitset_union(component_extension->c, residual_extension->c, last_node->c);
                residual_extension = advance_and_free_extension(residual_extension);
            }
            component_extension = advance_and_free_extension(component_extension);
        } else {
            // move the component_extension node to the resulting list
            last_node->next = component_extension;
            last_node = component_extension;
            component_extension = component_extension->next;
            last_node->next = NULL;
        }
    }
    free_bitset(component); 

    ListNode* first_extension = head->next;
    free_list_node(head);
    // TODO: Hashing
    return first_extension;
}

ListNode* ee_pr_scc(AF* af, ListNode* (*preferred_extensions)(AF* af))
{
    Map subextensions;
    MAP_INIT(&subextensions, MAP_SIZE);
    BitSet* all_arguments = create_bitset(af->size);
    set_bitset(all_arguments);
    BitSet* no_arguments = create_bitset(af->size);
    PAF* paf = af2paf(af);
    ListNode* first_extension = compute_preferred_extensions(paf, 
                                                             // all_arguments,
                                                             // no_arguments, 
                                                             preferred_extensions,
                                                             &subextensions);
    // TODO: free subextensions (without destroying extensions in the returned list)
    free_bitset(all_arguments);
    free_bitset(no_arguments);
    free_paf(paf, false);
    return first_extension;
}

#endif // MAP_SIZE