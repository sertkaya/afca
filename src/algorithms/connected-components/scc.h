#ifndef AF_SCC_H_
#define AF_SCC_H_

#include "../../af/af.h"
#include "../../utils/linked_list.h"


ListNode* scc_stable_extensions(AF* af, ListNode* (*stable_extensions)(AF* af));

#endif /* AF_SCC_H_ */
