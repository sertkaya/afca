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

#ifndef AF_STABLE_EXTENSIONS_NC_H_
#define AF_STABLE_EXTENSIONS_NC_H_

#include "../../af/af.h"
#include "../../utils/array_list.h"

// Computes all stable extensions and puts them into result
ArrayList* ee_st_next_closure(AF* af);

// Computes a single stable extension if there are any, and puts it into result
void *se_st_next_closure(AF* af, BitSet* result);

// Computes a stable extension containging arg (if there is any), and put it into result
void dc_st_next_closure(AF* af, int argument, BitSet* result);

#endif /* AF_STABLE_EXTENSIONS_NC_H_ */
