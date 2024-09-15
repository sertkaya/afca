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

#include "af/sort.h"
#include "algorithms/next-closure/stable.h"
#include "algorithms/norris/stable.h"
#include "algorithms/nourine/stable.h"
#include "algorithms/connected-components/cc.h"
#include "parser/af_parser.h"
#include "utils/timer.h"

int main(int argc, char *argv[]) {
	int c;
	bool problem_flag = 0, algorithm_flag = 0, input_flag = 0, output_flag = 0, wrong_argument_flag = 0, verbose_flag = 0, sort_flag = 0;
	char *problem = "", *algorithm = "", *af_file_name = "", *output_file = "";
	int sort_type = 0;
	static char usage[] = "Usage: %s -a [next-closure | norris | nourine | scc-norris | wcc-norris | scc-nourine | wcc-nourine] -p [SE-ST, EE-ST] -f input -o output\n";

	while ((c = getopt(argc, argv, "a:p:f:o:v:s")) != -1)
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
		case 's':
			sort_flag = 1;
			// sort_type = atoi(optarg);
			break;
		case '?':
			wrong_argument_flag = 1;
			break;
		}
	if (wrong_argument_flag || !input_flag || !output_flag || !algorithm_flag || !problem_flag) {
		fprintf(stderr, usage, argv[0]);
		exit(EXIT_FAILURE);
	}

	enum alg_type {NEXT_CLOSURE , NORRIS, NOURINE, SCC_NORRIS, WCC_NORRIS, SCC_NOURINE, WCC_NOURINE};
	enum alg_type alg;
	if (strcmp(algorithm, "next-closure") ==0)
		alg = NEXT_CLOSURE;
	else if (strcmp(algorithm, "norris") ==0)
		alg = NORRIS;
	else if (strcmp(algorithm, "nourine") ==0)
		alg = NOURINE;
	else if (strcmp(algorithm, "scc-norris") ==0)
		alg = SCC_NORRIS;
	else if (strcmp(algorithm, "wcc-norris") ==0)
		alg = WCC_NORRIS;
	else if (strcmp(algorithm, "scc-nourine") ==0)
		alg = SCC_NOURINE;
	else if (strcmp(algorithm, "wcc-nourine") ==0)
		alg = WCC_NOURINE;
	else {
		fprintf(stderr, "Unknown algorithm %s\n", algorithm);
		fprintf(stderr, usage, argv[0]);
		exit(EXIT_FAILURE);
	}

	enum prob_type {EE_ST, SE_ST, CE_ST};
	enum prob_type prob;
	if (strcmp(problem, "EE-ST") == 0)
		prob = EE_ST;
	else if (strcmp(problem, "SE-ST") == 0)
		prob = SE_ST;
	else if (strcmp(problem, "CE-ST") == 0)
		prob = CE_ST;
	else {
		fprintf(stderr, "Unknown problem %s\n", problem);
		fprintf(stderr, usage, argv[0]);
		exit(EXIT_FAILURE);
	}

	// open the af file
	FILE* input_fd;
	FILE* output;
	input_fd = fopen(af_file_name, "r");
	assert(input_fd != NULL);

	struct timeval start_time, stop_time;
	START_TIMER(start_time);

	// Read the file into an argumentation framework.
	AF *input_af = read_af(input_fd);
	fclose(input_fd);

	STOP_TIMER(stop_time);
	printf("Parsing time: %.3f milisecs\n", TIME_DIFF(start_time, stop_time) / 1000);

	// Sort the af
	AF *af = input_af;
	if (sort_flag) {
		af = sort_af(input_af);
	}
	// TODO: free the input_af? Needed for mapping back the sorted af?


	if (verbose_flag) {
		// print_short_stats(kb);
	}

	// open the output file
	output = fopen(output_file, "w");
	assert(output != NULL);

	START_TIMER(start_time);

	switch(prob) {
		case EE_ST:
			switch (alg) {
				case NEXT_CLOSURE:
					ee_st_next_closure(af, output);
					break;
				case NORRIS:
					ee_st_norris(af, output);
					break;
				case NOURINE:
					ee_st_nourine(af, output);
					break;
				case SCC_NORRIS:
					run_cc_norris(af, output, true);
					break;
				case WCC_NORRIS:
					run_cc_norris(af, output, false);
					break;
				case SCC_NOURINE:
					run_cc_nourine(af, output, true);
					break;
				case WCC_NOURINE:
					run_cc_nourine(af, output, false);
					break;
			}
			break;
		case SE_ST:
			switch (alg) {
				case NEXT_CLOSURE:
					BitSet *st_ext = se_st_next_closure(af);
					print_set(st_ext, output, "\n");
					BitSet *x = map_indices_back(st_ext);
					print_set(x, output, "\n");
					free_bitset(st_ext);
					free_bitset(x);
					break;
				case NORRIS:
					se_st_norris(af, output);
					break;
				case NOURINE:
					se_st_nourine(af, output);
					break;
				case SCC_NORRIS:
				case WCC_NORRIS:
				case SCC_NOURINE:
				case WCC_NOURINE:
					fprintf(stderr, "Problem %s is not supported with algorithm %s.\n", problem, algorithm);
					fclose(output);
					exit(EXIT_FAILURE);
			}
			break;
		case CE_ST:
			switch (alg) {
				case NORRIS:
					run_scc_norris_count(af, output);
					break;
				case NEXT_CLOSURE:
				case NOURINE:
				case SCC_NORRIS:
				case WCC_NORRIS:
				case SCC_NOURINE:
				case WCC_NOURINE:
					fprintf(stderr, "Problem %s is not supported with algorithm %s.\n", problem, algorithm);
					fclose(output);
					exit(EXIT_FAILURE);
			}
			break;
	}

	STOP_TIMER(stop_time);
	printf("Computation time: %.3f milisecs\n", TIME_DIFF(start_time, stop_time) / 1000);

	// close the output file
	fclose(output);

	return(0);
}

