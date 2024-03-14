#include "../../af/af.h"
#include "../utils/linked_list.h"
#include "../utils/map.h"


#define MAP_SIZE 	101


void dfs(AF* af, unsigned short i, BitSet* arguments, BitSet* visited) {
    SET_BIT(visited, i);
    // assume that vertex 0 is already visited
    for (unsigned short j = 1; j < af->size; ++j) { 
        if (TEST_BIT(arguments, j) && 
            !TEST_BIT(visited, j) && 
            TEST_BIT(af->graph[i], j)) {
            dfs(af, j, arguments, visited);
        }
    }
}


void backward_dfs(AF* af,
                  unsigned short i,
                  BitSet* arguments, 
                  BitSet* visited) {
    SET_BIT(visited, i);
    for (unsigned short j = 0; j < af->size; ++j) {
        if (TEST_BIT(arguments, j) && 
            !TEST_BIT(visited, j) && 
            TEST_BIT(af->graph[j], i)) {
            backward_dfs(af, j, arguments, visited);
        }
    }
}


// find a source component in af's subframework induced by arguments
// arguments must be non-empty
// component must be empty
void find_source_component(AF* af, BitSet* arguments, BitSet* component) {
    // find a vertex in a source component
    unsigned short source;
    for (unsigned short i = 0; i < af->size; ++i) {
        if (TEST_BIT(arguments, i) && !TEST_BIT(component, i)) {
            source = i;
            dfs(af, i, arguments, component);
        }
    }

    // extract the component containing source
    printf("Source vertex: %d\n", source);
    reset_bitset(component);
    backward_dfs(af, source, arguments, component);
}


void restore_base_indices(ListNode* node, PAF* paf, unsigned short base_size) {
    while (node) {
        BitSet* projected_extension = node->c;
        node->c = project_back(node->c, paf, base_size);
        free_bitset(projected_extension);
        node = node->next;
    }
}


void free_extension_node(ListNode* node){
    free_bitset((BitSet*) node->c);
    free_node(node);
}


ListNode* advance_and_free_extension(ListNode* node) {
    ListNode* next = node->next;
    free_extension_node(node);
    return next;
}


void log_set(BitSet* bs) {
	for (unsigned short i = 0; i < bs->size; ++i)
		if (TEST_BIT(bs, i))
			printf("%d", 1);
		else
			printf("%d", 0);
    printf("\n");
}


BitSet* extract_residual_arguments(AF* af, BitSet* arguments, BitSet* source_component, BitSet* component_extension) {
    BitSet* remainder = create_bitset(af->size);
    // add arguments outside source_component not attacked by component_extension
    bool something_remains = false;
	for (unsigned short i = 0; i < af->size; ++i) {
		if (TEST_BIT(arguments, i) && !TEST_BIT(source_component, i) && !check_set_attacks_arg(af, component_extension, i)) {
            SET_BIT(remainder, i);
            something_remains = true;
        }
	}
    if (!something_remains) {
        free_bitset(remainder);
        remainder = NULL;
    }
	return remainder;
}


ListNode* get_component_extensions(AF* af,
                                   BitSet* component, 
                                   ListNode* (*stable_extensions)(AF* af), 
                                   Map* subextensions) {
    ListNode* component_extension = MAP_GET(component, subextensions); 
    if (!component_extension) {
        PAF* projection = project_argumentation_framework(af, component);
        printf("Arguments in source component: %d\n", projection->af->size);
        component_extension = stable_extensions(projection->af);
        if (projection->af->size < af->size) {
            restore_base_indices(component_extension, projection, af->size);
        }
        free_projected_argumentation_framework(projection);
        printf("PUT %llu, %d, %llu\n", get_key(component), subextensions->bucket_count, get_key(component) % subextensions->bucket_count);
        BitSet* key = create_bitset(component->size);
        copy_bitset(component, key);
        MAP_PUT(key, component_extension, subextensions);
    } else {
        printf("Solution for component is taken from the hash\n");
    }
    return component_extension;
}


// compute the stable extensions in the subframework of af induced by arguments
ListNode* compute_extensions(AF* af,
                             BitSet* arguments, 
                             ListNode* (*stable_extensions)(AF* af),
                             Map* subextensions) {
    printf("\nARGUMENTS: %d\n", count_bits(arguments)); 
    log_set(arguments);
    printf("Hashed lists: %d\n", subextensions->element_count);
    ListNode* first_extension = MAP_GET(arguments, subextensions);
    if (first_extension) {
        printf("Solution is taken from the hash\n");
        return first_extension;
    }

    BitSet* component = create_bitset(af->size);
    find_source_component(af, arguments, component);

    ListNode* component_extension = get_component_extensions(af, component, stable_extensions, subextensions);
    if (count_bits(component) == count_bits(arguments)) {
        free_bitset(component);
        return component_extension;
    }

    ListNode* head = create_node(NULL);
    ListNode* last_node = head;

    while (component_extension) {
        printf("Arguments in component extension: %d\n", count_bits(component_extension->c));
        // log_set(component_extension->c);
        BitSet* residual_arguments = extract_residual_arguments(af, arguments, component, component_extension->c);
        if (residual_arguments) {
            printf("Residual arguments: %d\n", count_bits(residual_arguments));
            ListNode* residual_extension = compute_extensions(af, residual_arguments, stable_extensions, subextensions);
            // free_bitset(residual_arguments);

            while (residual_extension) {
                // printf("Component extension: ");
                // log_set(residual_extension->c);
                last_node->next = create_node(create_bitset(af->size));
                last_node = last_node->next;
                bitset_union(component_extension->c, residual_extension->c, last_node->c);
                // residual_extension = advance_and_free_extension(residual_extension);
                residual_extension = residual_extension->next;
            }
            // component_extension = advance_and_free_extension(component_extension);
            component_extension = component_extension->next;
        } else {
            printf("Residual arguments: 0\n");
            last_node->next = component_extension;
            last_node = last_node->next;
            component_extension = component_extension->next;
            last_node->next = NULL;
        }
    }
    free_bitset(component); 

    first_extension = head->next;
    free_node(head);
    MAP_PUT(arguments, first_extension, subextensions);
    return first_extension;
}


void free_map_contents(Map* subextensions) {

}

ListNode* scc_stable_extensions(AF* af, ListNode* (*stable_extensions)(AF* af)) {
    // maps a subset of arguments to a list of stable extensions of the induced argumentation subframework
    Map subextensions;
    MAP_INIT(&subextensions, MAP_SIZE);
    BitSet* all_arguments = create_bitset(af->size);
    set_bitset(all_arguments);
    ListNode* first_extension = compute_extensions(af, all_arguments, stable_extensions, &subextensions);
    // free_map_content(&subextensions);
    return first_extension;
}
