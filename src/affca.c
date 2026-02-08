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

#include "algorithms/complete.h"
#include "algorithms/stable.h"
#include "algorithms/preferred.h"
#include "validator/validator.h"
#include "parser/af_parser.h"
#include "utils/timer.h"


#define EXTENSION_TYPE_COUNT 3
enum extension_types {ST, PR, CO};

#define DECISION_PROBLEM_TYPE_COUNT 1
enum decision_problem_types {DC};

#define ENUMERATION_PROBLEM_TYPE_COUNT 2
enum problem_types {EE, SE};

void print_not_supported(char* problem, char* algorithm, FILE* output) {
	fprintf(stderr, "Problem %s is not supported with algorithm %s.\n", problem, algorithm);
	fclose(output);
	exit(EXIT_FAILURE);
}

void unsupported_feature(char* problem, char* extension) {
	fprintf(stderr, "Problem %s-%s is not supported\n", problem, extension);
}

int main(int argc, char *argv[]) {
	int c;

	bool problem_flag = 0, input_flag = 0, output_flag = 0, wrong_argument_flag = 0, argument_flag = 0, validate_flag = 0, extension_flag = 0, semantic_flag = 0;
	char *prob= "", *af_file_name = "", *output_file = "", *extension_file_name = "", *semantic_name;
	ARG_TYPE argument = 0;
	static char usage_solver[] = "Usage: %s -p [DC-ST, SE-ST, EE-ST, DC-PR, SE-PR, EE-PR, DC-CO, EE-CO] -a argument -f input -o output\n";
	static char usage_validator[] = "Usage: %s  -s [ST, PR, CO, AD] -f input -e extension\n";

	while ((c = getopt(argc, argv, "vp:o:f:e:s:a:")) != -1) {
		switch (c) {
			case 'p':
				problem_flag = 1;
			prob = optarg;
			break;
			case 'f':
				input_flag = 1;
				af_file_name = optarg;
			break;
			case 'o':
				output_flag = 1;
				output_file = optarg;
			break;
			case 'a':
				argument_flag = 1;
				argument = atoi(optarg);
			break;
			// for validation
			case 'v':
				validate_flag = 1;
			break;
			case 'e':
				extension_flag = 1;
				extension_file_name = optarg;
			break;
			case 's':
				semantic_flag = 1;
				semantic_name = optarg;
			break;
			case '?':
				wrong_argument_flag = 1;
			break;
		}
	}

	if (validate_flag) {
		if (!input_flag || !extension_flag || !semantic_flag) {
			fprintf(stderr, usage_validator, argv[0]);
			exit(EXIT_FAILURE);
		} else {
			// validate
			bool valid = validate(af_file_name, extension_file_name, semantic_name);
			if (valid) {
				printf("YES\n");
				return(0);
			}
			else {
				printf("NO\n");
				return (1);
			}
		}
	}

	if (wrong_argument_flag || !input_flag || !problem_flag) {
		fprintf(stderr, usage_solver, argv[0]);
		exit(EXIT_FAILURE);
	}

	FILE* output;
	if (output_flag) {
		// open the output file
		output = fopen(output_file, "w");
		assert(output != NULL);
	}
	else
		output = stdout;

	char *prob_type = strtok(prob, "-");
	char *ext_type = strtok(NULL, "-");

	enum problem_types problem_type;
	bool decision_problem = false;
	// enumeration problems
	if (strcmp(prob_type, "EE") == 0)
		problem_type = EE;
	else if (strcmp(prob_type, "SE") == 0)
		problem_type = SE;
	//decision problems
	else if (strcmp(prob_type, "DC") == 0) {
		decision_problem = true;
		problem_type = DC;
		if (!argument_flag) {
			fprintf(stderr, usage_solver, argv[0]);
			exit(EXIT_FAILURE);
		}
	}
	else {
		fprintf(stderr, "Unknown problem type %s\n", prob_type);
		fprintf(stderr, usage_solver, argv[0]);
		exit(EXIT_FAILURE);
	}

	enum extension_types extension_type;
	if (strcmp(ext_type, "ST") == 0)
		extension_type = ST;
	else if (strcmp(ext_type, "PR") == 0)
		extension_type = PR;
	else if (strcmp(ext_type, "CO") == 0)
		extension_type = CO;
	else {
		fprintf(stderr, "Unknown extension type %s\n", ext_type);
		fprintf(stderr, usage_solver, argv[0]);
		exit(EXIT_FAILURE);
	}

	// open the af file
	FILE* input_fd;
	input_fd = fopen(af_file_name, "r");
	assert(input_fd != NULL);

	struct timeval start_time, stop_time;

	START_TIMER(start_time);
	// Read the file into an argumentation framework.
	AF *input_af = read_af(input_fd);
	fclose(input_fd);
	STOP_TIMER(stop_time);
	printf("Parsing time: %.3f milisecs\n", TIME_DIFF(start_time, stop_time) / 1000);

	// matrix of functions for decision problems.
	// Function prototype: List* f(AF*, ARG_TYPE)
    // ArrayList* (*decision_functions[DECISION_PROBLEM_TYPE_COUNT][EXTENSION_TYPE_COUNT]) (AF*, ARG_TYPE);
    bool* (*decision_functions[DECISION_PROBLEM_TYPE_COUNT][EXTENSION_TYPE_COUNT]) (AF*, ARG_TYPE);
	for (int i = 0; i < DECISION_PROBLEM_TYPE_COUNT; ++i)
		for (int j = 0; j < EXTENSION_TYPE_COUNT; ++j)
			decision_functions[i][j] = NULL;

	decision_functions[DC][CO] = &dc_co;
	decision_functions[DC][ST] = &dc_st;
	// ...
	// ...

	// matrix of functions for enumeration problems.
	// Function prototype: List* f(AF*)
    // ArrayList* (*enumeration_functions[ENUMERATION_PROBLEM_TYPE_COUNT][EXTENSION_TYPE_COUNT]) (AF*);
    bool* (*enumeration_functions[ENUMERATION_PROBLEM_TYPE_COUNT][EXTENSION_TYPE_COUNT]) (AF*);
	for (int i = 0; i < ENUMERATION_PROBLEM_TYPE_COUNT; ++i)
		for (int j = 0; j < EXTENSION_TYPE_COUNT; ++j)
				enumeration_functions[i][j] = NULL;
	enumeration_functions[SE][PR] = &se_pr;

	START_TIMER(start_time);
	// ArrayList* extension = NULL;
	bool* extension = NULL;
	if (decision_problem) {
		// arguments are internally indices
		--argument;
		// decision problem
		if (decision_functions[problem_type][extension_type] == NULL)
			unsupported_feature(prob_type,ext_type);
		else
			extension = decision_functions[problem_type][extension_type](input_af, argument);
		if (extension == NULL)
			fprintf(output, "NO\n");
		else {
			fprintf(output, "YES\nw ");
			// print_list(output, extension,"\n");
			for (int i = 0; i < input_af->size; ++i) {
				if (extension[i])
					fprintf(output, "%d ", i+1);
			}
			fprintf(output, "\n");
		}
	}
	else {
		// enumeration problem
		if (enumeration_functions[problem_type][extension_type] == NULL)
			unsupported_feature(prob_type,ext_type);
		else
			extension = enumeration_functions[problem_type][extension_type](input_af);
	}
	STOP_TIMER(stop_time);
	printf("Computation time: %.3f milisecs\n", TIME_DIFF(start_time, stop_time) / 1000);

	free_argumentation_framework(input_af);
	// close the output file
	fclose(output);

	return(0);
}

