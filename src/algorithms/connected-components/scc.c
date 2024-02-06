#include "../../af/af.h"
#include "../utils/linked_list.h"


void reset_bool_array(bool* array, unsigned short size) {
    for (unsigned short i = 0; i < size; ++i) {
        array[i] = false;
    }
}


void dfs(AF* af, int i, bool* visited) {
    visited[i] = true;
    for (unsigned short j = 1; j < af->size; ++j) {
        if (!visited[j] && TEST_BIT(af->graph[i], j)) {
            dfs(af, j, visited);
        }
    }
}


void backward_dfs(AF* af, int i, bool* visited) {
    visited[i] = true;
    for (unsigned short j = 1; j < af->size; ++j) {
        if (!visited[j] && TEST_BIT(af->graph[j], i)) {
            backward_dfs(af, j, visited);
        }
    }
}

void find_source_component(AF* af, bool* component) {

    // find a vertex in a source component
    reset_bool_array(component, af->size);
    unsigned short source;
    for (unsigned short i = 0; i < af->size; ++i) {
        if (!component[i]) {
            source = i;
            dfs(af, i, component);
        }
    }

    // extract the component containg vertex i
    printf("Source vertex: %d\n", source);
    reset_bool_array(component, af->size);
    backward_dfs(af, source, component);
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


unsigned short len_set(BitSet* bs) {
    unsigned short len = 0;
	for (unsigned short i = 0; i < bs->size; ++i) {
		if (TEST_BIT(bs, i)) {
            ++len;
        }
    }
    return len;
}


PAF* extract_residual_framework(AF* af, bool* source_component, BitSet* component_extension) {
	bool remainder[af->size];
    // add arguments outside source_component not attacked by component_extension
	for (unsigned short i = 0; i < af->size; ++i) {
		remainder[i] = !source_component[i] && !check_set_attacks_arg(af, component_extension, i);
	}
	return project_argumentation_framework(af, remainder);
}



ListNode* scc_stable_extensions(AF* af, ListNode* (*stable_extensions)(AF* af)) {
    printf("\nARGUMENTS: %d\n", af->size);
    bool component[af->size];
    find_source_component(af, component);

    PAF* projection = project_argumentation_framework(af, component);
    printf("Arguments in source component: %d\n", projection->af->size);
    ListNode* component_extension = stable_extensions(projection->af);
    if (projection->af->size == af->size) {
        free_projected_argumentation_framework(projection);
        return component_extension;
    }
    restore_base_indices(component_extension, projection, af->size);
    free_projected_argumentation_framework(projection);

    ListNode* head = create_node(NULL);
    ListNode* last_node = head;

    while (component_extension) {
        printf("Arguments in component extension: %d\n", len_set(component_extension->c));
        // log_set(component_extension->c);
        PAF* residual_framework = extract_residual_framework(af, component, component_extension->c);
        printf("Residual arguments: %d\n", residual_framework->af->size);
        ListNode* residual_extension = scc_stable_extensions(residual_framework->af, stable_extensions);
        restore_base_indices(residual_extension, residual_framework, af->size);
        free_projected_argumentation_framework(residual_framework);

        while (residual_extension) {
            // printf("Component extension: ");
            // log_set(residual_extension->c);

            last_node->next = create_node(create_bitset(af->size));
            last_node = last_node->next;
            bitset_union(component_extension->c, residual_extension->c, last_node->c);
            residual_extension = advance_and_free_extension(residual_extension);
        }

        component_extension = advance_and_free_extension(component_extension);
    }

    ListNode* first_extension = head->next;
    free_node(head);
    return first_extension;
}
