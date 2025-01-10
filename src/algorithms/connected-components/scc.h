#ifndef AF_SCC_H_
#define AF_SCC_H_

#include "../../af/af.h"
#include "../../utils/linked_list.h"


ListNode* scc_stable_extensions(AF* af, ListNode* (*stable_extensions)(AF* af));

size_t scc_count_stable_etensions(AF* af, ListNode* (*stable_extensions)(AF* af));

ListNode* ee_pr_scc(AF* af, ListNode* (*preferred_extensions)(AF* af));

void restore_base_indices(ListNode* node, PAF* paf);
void find_source_component(AF* af, BitSet* arguments, BitSet* component);
BitSet* extract_residual_arguments(AF* af, BitSet* arguments, BitSet* source_component, BitSet* component_extension);

void backward_dfs(AF* af, SIZE_TYPE i, BitSet* arguments, BitSet* visited);

#endif /* AF_SCC_H_ */
