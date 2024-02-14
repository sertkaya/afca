#ifndef AF_WCC_H_
#define AF_WCC_H_

#include "../../af/af.h"
#include "../../utils/linked_list.h"


ListNode* wcc_stable_extensions(AF* af, ListNode* (*stable_extensions)(AF* af));

#endif /* AF_WCC_H_ */
