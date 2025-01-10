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

#ifndef CC_H
#define CC_H


// stable extensions
void run_scc_norris_count(AF* af, FILE* output);

void run_cc(AF* af, ListNode* (*stable_extensions)(AF* af), FILE* output, bool scc);

void run_cc_mis(AF* af, FILE* output, bool scc);

void run_cc_norris(AF* af, FILE* output, bool scc);

void run_cc_norris_bu(AF* af, FILE* output, bool scc);

void run_cc_nourine(AF* af, FILE* output, bool scc);

// preferred extensions
void run_cc_preferred(AF* af, ListNode* (*preferred_extensions)(AF* af), FILE* output);
void run_scc_next_closure(AF* af, FILE* output);

#endif //CC_H
