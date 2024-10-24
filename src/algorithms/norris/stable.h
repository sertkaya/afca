/*
 * AFCA - argumentation framework using closed sets
 *
 * Copyright (C) Baris Sertkaya (sertkaya@fb2.fra-uas.de)
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

#ifndef AF_STABLE_EXTENSIONS_NORRIS_H_
#define AF_STABLE_EXTENSIONS_NORRIS_H_

#include "../../af/af.h"
#include "../../utils/linked_list.h"

ListNode* ee_st_norris(AF* af);

void se_st_norris(AF* af, FILE *outfile);

ListNode* enumerate_stable_extensions_norris(AF* af);

ListNode* enumerate_stable_extensions_norris_bottom_up(AF* af);

void run_norris_bu(AF* af, FILE* output);


#endif /* AF_STABLE_EXTENSIONS_NORRIS_H_ */
