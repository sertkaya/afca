#include "scc.h"
#include "../utils/map.h"


#define MAP_SIZE 	101


void free_extension_list(ListNode* node){
    while (node) {
        ListNode* next = node->next;
        free_bitset((BitSet*) node->c);
        free_list_node(node);
        node = next;
    }
}


ListNode* get_component_extensions_for_counting(AF* af,
                                                BitSet* component, 
                                                ListNode* (*stable_extensions)(AF* af), 
                                                Map* subextensions,
                                                Map* subextension_counts) {
    ListNode* component_extension = MAP_GET(component, subextensions); 
    if (!component_extension) {
        PAF* projection = project_argumentation_framework(af, component);
        component_extension = stable_extensions(projection->af);
        if (projection->af->size < af->size) {
            restore_base_indices(component_extension, projection, af->size);
        }
        free_projected_argumentation_framework(projection);
        BitSet* key = create_bitset(component->size);
        copy_bitset(component, key);
        MAP_PUT(key, component_extension, subextensions);
        size_t* pCount = calloc(sizeof(size_t), 1);
        *pCount = count_nodes(component_extension);
        MAP_PUT(key, pCount, subextension_counts);
    }
    return component_extension;
}


// count the stable extensions in the subframework of af induced by arguments
size_t count_extensions(AF* af,
                        BitSet* arguments, 
                        ListNode* (*stable_extensions)(AF* af), 
                        Map* subextensions,
                        Map* subextension_counts) {
    size_t* pcount = MAP_GET(arguments, subextension_counts);
    if (pcount) {
        return *pcount;
    }

    BitSet* component = create_bitset(af->size);
    find_source_component(af, arguments, component);

    ListNode* component_extension = get_component_extensions_for_counting(af, 
                                                                          component,
                                                                          stable_extensions, 
                                                                          subextensions, 
                                                                          subextension_counts);

    size_t n = 0;
    if (count_bits(component) == count_bits(arguments)) {
        size_t* pcount = MAP_GET(component, subextension_counts);
        n = *pcount;
    } else {
        while (component_extension) {
            BitSet* residual_arguments = extract_residual_arguments(af, arguments, component, component_extension->c);
            n += residual_arguments ? count_extensions(af,
                                                    residual_arguments,
                                                    stable_extensions,
                                                    subextensions,
                                                    subextension_counts) :
                                    1;
            // TODO: free_bitset(residual_arguments);
            component_extension = component_extension->next;
        }
    }
    free_bitset(component);
    free_extension_list(component_extension);

    return n;
}


size_t scc_count_stable_etensions(AF* af, ListNode* (*stable_extensions)(AF* af)) {
    Map subextensions;
    MAP_INIT(&subextensions, MAP_SIZE);
    Map subextension_counts;
    MAP_INIT(&subextension_counts, MAP_SIZE);
    BitSet* all_arguments = create_bitset(af->size);
    set_bitset(all_arguments);
    return count_extensions(af, all_arguments, stable_extensions, &subextensions, &subextension_counts);
}
