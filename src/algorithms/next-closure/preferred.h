#ifndef AF_PREFERRED_EXTENSIONS_NC_H_
#define AF_PREFERRED_EXTENSIONS_NC_H_

#include "../../af/af.h"
#include "../../utils/list.h"

// Computes all preferred extensions and puts them into result
List* ee_pr_next_closure(AF* af);

#endif /* AF_PREFERRED_EXTENSIONS_NC_H_ */
