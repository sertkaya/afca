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
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <getopt.h>

#include "algorithms/next-closure/stable.h"
#include "algorithms/norris/stable.h"
#include "algorithms/nourine/stable.h"
#include "algorithms/connected-components/wcc.h"
#include "algorithms/connected-components/scc.h"
#include "parser/af_parser.h"
#include "utils/timer.h"


void print_extension(BitSet* ext, FILE* outfile) {
	print_set(ext, outfile, "\n");
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


void run_cc_norris(AF* af, FILE* output, bool scc) {
	run_cc(af, enumerate_stable_extensions_norris, output, scc);
}


void run_cc_nourine(AF* af, FILE* output, bool scc) {
	run_cc(af, enumerate_stable_extensions_via_implications, output, scc);
}


void usage(char* program) {
	fprintf(stderr, "Usage: %s -i input-file -o output-file -c\n", program);
}

int main(int argc, char *argv[]) {
	FILE* input_fd;
	FILE* output;

	int c;
	bool problem_flag = 0, algorithm_flag = 0, input_flag = 0, output_flag = 0, wrong_argument_flag = 0, verbose_flag = 0;
	char *problem = "", *algorithm = "", *af_file_name = "", *output_file = "";
	static char usage[] = "Usage: %s -a [next-closure | norris | nourine | scc-norris | wcc-norris | scc-nourine | wcc-nourine] -p [SE-ST, EE-ST] -f input -o output\n";

	while ((c = getopt(argc, argv, "a:p:f:o:v")) != -1)
		switch (c) {
		case 'a':
			algorithm_flag = 1;
			algorithm = optarg;
			break;
		case 'p':
			problem_flag = 1;
			problem = optarg;
			break;
		case 'f':
			input_flag = 1;
			af_file_name = optarg;
			break;
		case 'o':
			output_flag = 1;
			output_file = optarg;
			break;
		case 'v':
			verbose_flag = 1;
			break;
		case '?':
			wrong_argument_flag = 1;
			break;
		}
	if (wrong_argument_flag || !input_flag || !output_flag || !algorithm_flag || !problem_flag) {
		fprintf(stderr, usage, argv[0]);
		exit(EXIT_FAILURE);
	}

	if ((strcmp(algorithm, "next-closure") != 0 &&
			strcmp(algorithm, "norris") != 0 &&
			strcmp(algorithm, "nourine") != 0 &&
			strcmp(algorithm, "wcc-norris") != 0 &&
			strcmp(algorithm, "scc-norris") != 0 &&
			strcmp(algorithm, "scc-nourine") != 0) ||
			(strcmp(problem, "SE-ST") != 0 && strcmp(problem, "EE-ST") != 0)) {
		fprintf(stderr, usage, argv[0]);
		exit(EXIT_FAILURE);
	}

	// open the af file
	input_fd = fopen(af_file_name, "r");
	assert(input_fd != NULL);

	struct timeval start_time, stop_time;
	START_TIMER(start_time);

	// Read the file into an argumentation framework.
	AF *af = read_af(input_fd);

	STOP_TIMER(stop_time);
	printf("Parsing time: %.3f milisecs\n", TIME_DIFF(start_time, stop_time) / 1000);

	// read and parse the graph
	fclose(input_fd);

	if (verbose_flag) {
		// print_short_stats(kb);
	}

	// open the output file
	output = fopen(output_file, "w");
	assert(output != NULL);

	START_TIMER(start_time);

	// compute the extensions
	if (strcmp(algorithm, "norris") == 0) {
		if (strcmp(problem, "EE-ST") == 0) {
			incremental_stable_extensions_norris(af, output);
		} else if (strcmp(problem, "SE-ST") == 0) {
			one_stable_extension_norris(af, output);
		} else {
			wrong_argument_flag = 1;
		}
	} else if (strcmp(algorithm, "next-closure") == 0) {
		if (strcmp(problem, "EE-ST") == 0) {
			stable_extensions_nc(af, output);
		} else if (strcmp(problem, "SE-ST") == 0) {
			one_stable_extension_nc(af, output);
		} else {
			wrong_argument_flag = 1;
		}
	} else if (strcmp(algorithm, "nourine") == 0) {
		if (strcmp(problem, "EE-ST") == 0) {
			stable_extensions_nourine(af, output);
		} else if (strcmp(problem, "SE-ST") == 0) {
			one_stable_extension_nourine(af, output);
		} else {
			wrong_argument_flag = 1;
		}
	} else if (strcmp(algorithm, "wcc-norris") == 0) {
		if (strcmp(problem, "EE-ST") == 0) {
			run_cc_norris(af, output, false);
		} else {
			wrong_argument_flag = 1;
		}
	} else if (strcmp(algorithm, "scc-norris") == 0) {
		if (strcmp(problem, "EE-ST") == 0) {
			run_cc_norris(af, output, true);
		} else {
			wrong_argument_flag = 1;
		}
	} else if (strcmp(algorithm, "wcc-norris") == 0) {
		if (strcmp(problem, "EE-ST") == 0) {
			run_cc_nourine(af, output, false);
		} else {
			wrong_argument_flag = 1;
		}
	} else if (strcmp(algorithm, "scc-nourine") == 0) {
		if (strcmp(problem, "EE-ST") == 0) {
			run_cc_nourine(af, output, true);
		} else {
			wrong_argument_flag = 1;
		}
	}
	if (wrong_argument_flag) {
		fprintf(stderr, "Problem %s is not supported with algorithm %s.\n", problem, algorithm);
		fclose(output);
		exit(EXIT_FAILURE);
	}

	STOP_TIMER(stop_time);
	printf("Computation time: %.3f milisecs\n", TIME_DIFF(start_time, stop_time) / 1000);

	// close the output file
	fclose(output);

	return(0);
}

