#ifndef AF_IDEAL_EXTENSION_H_
#define AF_IDEAL_EXTENSION_H_

#include "../../af/af.h"
#include "../../bitset/bitset.h"
#include "../../utils/linked_list.h"

BitSet* se_id(AF* af, ListNode* (*preferred_extensions)(AF* af));

#endif /* AF_IDEAL_EXTENSION_H_ */
