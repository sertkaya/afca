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

#include <stdio.h>
#include <inttypes.h>

#include "../../af/af.h"
#include "../../bitset/bitset.h"
#include "../../utils/linked_list.h"
#include "../../utils/list.h"

ListNode* ee_st_nourine(AF* c);

void se_st_nourine(AF* c, BitSet* result);

ListNode* enumerate_stable_extensions_via_implications(AF* attacks);