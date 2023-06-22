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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <getopt.h>

#include "af/stable_extensions_nc.h"
#include "af/preferred_extensions_nc.h"
#include "af/stable_extensions_norris.h"
#include "parser/af_parser.h"
#include "fca/context.h"
#include "fca/sorted_context.h"
#include "utils/timer.h"


void usage(char* program) {
	fprintf(stderr, "Usage: %s -i input-file -o output-file -c\n", program);
}

int main(int argc, char *argv[]) {
	FILE* input_af;
	FILE* output;

	int c, problem_flag = 0, algorithm_flag = 0, input_flag = 0, output_flag = 0, wrong_argument_flag = 0, verbose_flag = 0;
	char *problem = "", *algorithm = "", *af_file_name = "", *output_file = "";
	static char usage[] = "Usage: %s -a[next-closure|norris] -p problem -f input -o output\n";

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
	if (input_flag == 0) {
		fprintf(stderr, "%s: Provide an input argumentation framework\n", argv[0]);
		fprintf(stderr, usage, argv[0]);
		exit(EXIT_FAILURE);
	}
	if (output_flag == 0) {
		fprintf(stderr, "%s: Provide an output file\n", argv[0]);
		fprintf(stderr, usage, argv[0]);
		exit(EXIT_FAILURE);
	}
	if (algorithm_flag == 0) {
		fprintf(stderr, "%s: Provide one of the algorithms: next-closure | norris\n", argv[0]);
		fprintf(stderr, usage, argv[0]);
		exit(EXIT_FAILURE);
	}
	if (strcmp(algorithm, "next-closure") != 0 && strcmp(algorithm, "norris") != 0) {
		fprintf(stderr, "%s: Provide one of the algorithms: next-closure | norris \n", argv[0]);
		fprintf(stderr, usage, argv[0]);
		exit(EXIT_FAILURE);
	}
	if (problem_flag == 0) {
		fprintf(stderr, "%s: Provide one of the problems: EE-ST | EE-PR\n", argv[0]);
		fprintf(stderr, usage, argv[0]);
		exit(EXIT_FAILURE);
	}
	if ((strcmp(problem, "EE-ST") != 0) && (strcmp(problem, "EE-PR") !=0) && (strcmp(problem, "SE-ST") !=0)) {
		fprintf(stderr, "%s: Provide one of the problems EE-ST | EE-PR | SE-ST \n", argv[0]);
		fprintf(stderr, usage, argv[0]);
		exit(EXIT_FAILURE);
	}
	if (wrong_argument_flag) {
		fprintf(stderr, usage, argv[0]);
		exit(EXIT_FAILURE);
	}
	// open the af file
	input_af = fopen(af_file_name, "r");
	assert(input_af != NULL);

	// Create an empty context.
	Context* context = create_context();

	struct timeval start_time, stop_time;
	START_TIMER(start_time);

	// Read the af into the context.
	read_af(input_af, context);

	STOP_TIMER(stop_time);
	printf("Parsing time: %.3f milisecs\n", TIME_DIFF(start_time, stop_time) / 1000);

	// read and parse the graph
	fclose(input_af);

	// sort the context
	// heuristic: number of attacked arguments from high to low
	Context *sc = sort_context(context);

	if (verbose_flag) {
		// print_short_stats(kb);
	}

	// open the output file
	output = fopen(output_file, "w");
	assert(output != NULL);

	START_TIMER(start_time);

	// compute the stable extensions
	if (strcmp(algorithm, "norris") == 0) {
		if (strcmp(problem, "EE-ST") == 0)
			incremental_stable_extensions_norris(context, output);
		else if (strcmp(problem, "SE-ST") == 0)
			// one_stable_extension_norris(context, output);
			one_stable_extension_norris(sc, output);
	}
	else if (strcmp(algorithm, "next-closure") == 0) {
		if (strcmp(problem, "EE-ST") == 0)
			all_stable_extensions_nc(context, output);
		else if (strcmp(problem, "EE-PR") == 0)
			all_preferred_extensions_nc(context, output);
		else if (strcmp(problem, "SE-ST") == 0)
			one_stable_extension_nc(context, output);
	}

	STOP_TIMER(stop_time);
	printf("Computation time: %.3f milisecs\n", TIME_DIFF(start_time, stop_time) / 1000);

	// close the output file
	fclose(output);

	return 0;
}

