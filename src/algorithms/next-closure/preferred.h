#ifndef AF_PREFERRED_EXTENSIONS_NC_H_
#define AF_PREFERRED_EXTENSIONS_NC_H_

#include "../../af/af.h"
#include "../../utils/linked_list.h"

// Computes all preferred extensions and puts them into result
ListNode* ee_pr_next_closure(AF* af);

#endif /* AF_PREFERRED_EXTENSIONS_NC_H_ */
