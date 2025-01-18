/*
 * AFCA - argumentation framework using closed sets
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef AF_COMPLETE_EXTENSIONS_NC_H_
#define AF_COMPLETE_EXTENSIONS_NC_H_

#include "../../utils/array_list.h"
#include "../../af/af.h"

// Computes all complete extensions and puts them into result
// ListNode* ee_co_next_closure(AF* af);

// Assuming arguments are sorted in descending order of victim count: -s 1 -d 1
// BitSet* dc_co_next_closure(AF* attacks, int argument);

// BitSet* dc_co_next_closure_2(AF* attacks, int argument);

// BitSet* dc_co_subgraph_next_closure(AF* attacks, int argument);
ArrayList* dc_co_subgraph(AF* attacks, ARG_TYPE argument);

void closure_semi_complete(AF* af, AF* af_t, ArrayList* s, ArrayList* r, bool *r_bv);

#endif /* AF_COMPLETE_EXTENSIONS_NC_H_ */
