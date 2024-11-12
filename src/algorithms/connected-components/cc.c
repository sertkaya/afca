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

#include <stdlib.h>

#include "../../bitset/bitset.h"
#include "../maximal-independent-sets/mis.h"
#include "../norris/stable.h"
#include "../nourine/stable.h"
#include "scc.h"
#include "wcc.h"


void print_extension(BitSet* ext, FILE* outfile) {
	print_set(ext, outfile, "\n");
}

void run_scc_norris_count(AF* af, FILE* output) {
	fprintf(output, "%lu", scc_count_stable_etensions(af, ee_st_norris));
	free_argumentation_framework(af);
}

void run_cc(AF* af, ListNode* (*stable_extensions)(AF* af), FILE* output, bool scc) {
	ListNode* head = scc ? scc_stable_extensions(af, stable_extensions) : wcc_stable_extensions(af, stable_extensions);
	ListNode* node = head;

	while (node) {
		print_extension(node->c, output);
		node = node->next;
	}

	while (head) {
		ListNode* next = head->next;
		free_bitset((BitSet*) head->c);
		free(head);
		head = next;
	}

	free_argumentation_framework(af);
}

void run_cc_mis(AF* af, FILE* output, bool scc) {
	run_cc(af, ee_st_maximal_independent_sets, output, scc);
}

void run_cc_norris(AF* af, FILE* output, bool scc) {
	run_cc(af, ee_st_norris, output, scc);
}


void run_cc_norris_bu(AF* af, FILE* output, bool scc) {
	run_cc(af, enumerate_stable_extensions_norris_bottom_up, output, scc);
}


void run_cc_nourine(AF* af, FILE* output, bool scc) {
	run_cc(af, enumerate_stable_extensions_via_implications, output, scc);
}

