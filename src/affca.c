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
#include "algorithms/dc.h"
#include "algorithms/cbo/preferred.h"
#include "algorithms/connected-components/scc.h"
#include "algorithms/ideal/ideal.h"
#include "algorithms/maximal-independent-sets/mis.h"
#include "algorithms/next-closure/preferred.h"
#include "algorithms/next-closure/stable.h"
#include "algorithms/next-closure/complete.h"
#include "algorithms/norris/stable.h"
#include "algorithms/nourine/stable.h"
#include "algorithms/connected-components/cc.h"
#include "parser/af_parser.h"
#include "utils/timer.h"


enum alg_type {CBO, MIS, NEXT_CLOSURE, NORRIS, NORRIS_BU, NOURINE, SCC_MIS, WCC_MIS, SCC_NEXT_CLOSURE, SCC_NORRIS, WCC_NORRIS, SCC_NORRIS_BU, SCC_NOURINE, WCC_NOURINE, SUBGRAPH, SUBGRAPH_ADJ};
enum prob_type {EE_ST, SE_ST, CE_ST, DC_ST, EE_PR, SE_PR, DC_PR, DS_PR, SE_ID, EE_CO, DC_CO};


ListNode* ee_pr_scc_next_closure(AF* af)
{
	return ee_pr_scc(af, ee_pr_next_closure);
}


void print_not_supported(char* problem, char* algorithm, FILE* output)
{
	fprintf(stderr, "Problem %s is not supported with algorithm %s.\n", problem, algorithm);
	fclose(output);
	exit(EXIT_FAILURE);
}


int main(int argc, char *argv[]) {
	int c;
	bool problem_flag = 0, algorithm_flag = 0, input_flag = 0, output_flag = 0, wrong_argument_flag = 0, verbose_flag = 0, sort_flag = 0, argument_flag = 0;
	char *problem = "", *algorithm = "", *af_file_name = "", *output_file = "";
	int sort_type = 0, sort_direction = 0, argument;
	static char usage[] = "Usage: %s -l [cbo | max-independent-sets | next-closure | norris | norris-bu | nourine | scc-max-independent-sets | wcc-max-independent-sets | scc-next-closure | scc-norris | scc-norris-bu | wcc-norris | scc-nourine | wcc-nourine | subgraph] "
					      "-p [SE-ST, EE-ST, DC-ST, EE-PR, SE-PR, DC-PR, DS-PR, SE-ID, EE-CO] -a argument -f input -o output\n";

	while ((c = getopt(argc, argv, "l:p:f:o:v:s:d:a:")) != -1)
		switch (c) {
		case 'l':
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
			sort_type = atoi(optarg);
			break;
		case 'd':
			sort_direction = atoi(optarg);
			break;
		case 'a':
			argument_flag = 1;
			argument = atoi(optarg);
			break;
		case '?':
			wrong_argument_flag = 1;
			break;
		}
	if (wrong_argument_flag || !input_flag || !output_flag || !algorithm_flag || !problem_flag) {
		fprintf(stderr, usage, argv[0]);
		exit(EXIT_FAILURE);
	}

	enum alg_type alg;
	if (strcmp(algorithm, "cbo") == 0) {
		alg = CBO;
	} else if (strcmp(algorithm, "mis") == 0) {
		alg = MIS;
	} else if (strcmp(algorithm, "next-closure") == 0) {
		alg = NEXT_CLOSURE;
	} else if (strcmp(algorithm, "norris") == 0) {
		alg = NORRIS;
	} else if (strcmp(algorithm, "nourine") == 0) {
		alg = NOURINE;
	} else if (strcmp(algorithm, "scc-mis") == 0) {
		alg = SCC_MIS;
	} else if (strcmp(algorithm, "wcc-mis") == 0) {
		alg = WCC_MIS;
	} else if (strcmp(algorithm, "scc-next-closure") == 0) {
		alg = SCC_NEXT_CLOSURE;
	} else if (strcmp(algorithm, "scc-norris") == 0) {
		alg = SCC_NORRIS;
	} else if (strcmp(algorithm, "wcc-norris") == 0) {
		alg = WCC_NORRIS;
	} else if (strcmp(algorithm, "norris-bu") == 0) {
		alg = NORRIS_BU;
	} else if (strcmp(algorithm, "scc-norris-bu") == 0) {
		alg = SCC_NORRIS_BU;
	} else if (strcmp(algorithm, "scc-nourine") == 0) {
		alg = SCC_NOURINE;
	} else if (strcmp(algorithm, "wcc-nourine") == 0) {
		alg = WCC_NOURINE;
	} else if (strcmp(algorithm, "subgraph") == 0) {
		alg = SUBGRAPH;
	} else if (strcmp(algorithm, "subgraph_adj") == 0) {
		alg = SUBGRAPH_ADJ;
	} else {
		fprintf(stderr, "Unknown algorithm %s\n", algorithm);
		fprintf(stderr, usage, argv[0]);
		exit(EXIT_FAILURE);
	}

	enum prob_type prob;
	if (strcmp(problem, "EE-ST") == 0)
		prob = EE_ST;
	else if (strcmp(problem, "SE-ST") == 0)
		prob = SE_ST;
	else if (strcmp(problem, "CE-ST") == 0)
		prob = CE_ST;
	else if (strcmp(problem, "DC-ST") == 0) {
		prob = DC_ST;
		if (!argument_flag) {
			fprintf(stderr, usage, argv[0]);
			exit(EXIT_FAILURE);
		}
	} else if (strcmp(problem, "EE-PR") == 0) {
		prob = EE_PR;
	} else if (strcmp(problem, "SE-PR") == 0) {
		prob = SE_PR;
	} else if (strcmp(problem, "DC-PR") == 0) {
		prob = DC_PR;
		if (!argument_flag) {
			fprintf(stderr, usage, argv[0]);
			exit(EXIT_FAILURE);
		}
	} else if (strcmp(problem, "DS-PR") == 0) {
		prob = DS_PR;
		if (!argument_flag) {
			fprintf(stderr, usage, argv[0]);
			exit(EXIT_FAILURE);
		}
	} else if (strcmp(problem, "SE-ID") == 0) {
		prob = SE_ID;
	} else if (strcmp(problem, "EE-CO") == 0) {
		prob = EE_CO;
	} else if (strcmp(problem, "DC-CO") == 0) {
		prob = DC_CO;
		if (!argument_flag) {
			fprintf(stderr, usage, argv[0]);
			exit(EXIT_FAILURE);
		}
	} else {
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
	int *mapping;
	if (sort_flag) {
		// START_TIMER(start_time);
		mapping = sort_af(input_af, af, sort_type, sort_direction);
		// STOP_TIMER(stop_time);
		// printf("Sorting time: %.3f milisecs\n", TIME_DIFF(start_time, stop_time) / 1000);
	}
	// TODO: free the input_af? Needed for mapping back the sorted af?


	if (verbose_flag) {
		// print_short_stats(kb);
	}

	// open the output file
	output = fopen(output_file, "w");
	assert(output != NULL);

	START_TIMER(start_time);

	// TODO: Think about a matrix with pointers to relevant functions.
	ListNode *result_list = NULL;
	switch(prob) {
		case EE_ST:
			switch (alg) {
				case MIS:
					result_list = ee_st_maximal_independent_sets(af);
					break;
				case NEXT_CLOSURE:
					result_list = ee_st_next_closure(af);
					break;
				case NORRIS:
					result_list = ee_st_norris(af);
					break;
				case NOURINE:
					result_list = ee_st_nourine(af);
					break;
				case SCC_MIS:
					run_cc_mis(af, output, true);
					break;
				case WCC_MIS:
					run_cc_mis(af, output, false);
					break;
				case SCC_NORRIS:
					run_cc_norris(af, output, true);
					break;
				case WCC_NORRIS:
					run_cc_norris(af, output, false);
					break;
				case NORRIS_BU:
					run_norris_bu(af, output);
					break;
				case SCC_NORRIS_BU:
					run_cc_norris_bu(af, output, true);
					break;
				case SCC_NOURINE:
					run_cc_nourine(af, output, true);
					break;
				case WCC_NOURINE:
					run_cc_nourine(af, output, false);
					break;
				default:
					fprintf(stderr, "Problem %s is not supported with algorithm %s.\n", problem, algorithm);
					fclose(output);
					exit(EXIT_FAILURE);			}
			if (sort_flag) {
				ListNode* node = result_list;
				while (node) {
					// map back the indices if af was sorted before
					BitSet *x = map_indices(node->c, mapping);
					print_set(x, output, "\n");
					node = node->next;
				}
			}
			else {
				print_list(result_list, (void (*)(void *, FILE*, const char*)) print_set, output);
			}
			free_list(result_list, (void (*)(void *)) free_bitset);
			break;
		case SE_ST: {
			BitSet *result_se = create_bitset(af->size);
			switch (alg) {
				case NEXT_CLOSURE:
					se_st_next_closure(af, result_se);
					break;
				case MIS:
					result_se = se_st_mis(af);
					break;
				case NORRIS:
					// se = se_st_norris(af, output);
					se_st_norris(af, output);
					break;
				case NOURINE:
					se_st_nourine(af, result_se);
					break;
				default:
					fprintf(stderr, "Problem %s is not supported with algorithm %s.\n", problem, algorithm);
					fclose(output);
					exit(EXIT_FAILURE);
			}
			if (sort_flag) {
				// map back the indices if af was sorted before
				BitSet *x = map_indices(result_se, mapping);
				print_set(x, output, "\n");
				free_bitset(x);
			}
			else {
				print_set(result_se, output, "\n");
			}
			free_bitset(result_se);
			break;
		}
		case DC_ST:
			// On the command line arguments are named starting from 1. Internally, they start from 0:
			--argument;
			BitSet *result_dc =  create_bitset(af->size);
			switch (alg) {
				case MIS:
					result_dc = dc(af, argument, se_st_mis);
					break;
				case NEXT_CLOSURE:
					dc_st_next_closure(af, argument, result_dc);
					break;
				default:
					fprintf(stderr, "Problem %s is not supported with algorithm %s.\n", problem, algorithm);
					fclose(output);
					exit(EXIT_FAILURE);
			}
		    if (!result_dc || bitset_is_emptyset(result_dc)) {
				// No stable extension containing argument
			    fprintf(output, "NO\n");
		    }
			else {
				if (sort_flag) {
					// map back the indices if af was sorted before
					BitSet *x = map_indices(result_dc, mapping);
					print_set(x, output, "\n");
					free_bitset(x);
				}
				else {
					print_set(result_dc, output, "\n");
				}
				free_bitset(result_dc);
			}
			break;
		case CE_ST:
			switch (alg) {
				case SCC_NORRIS:
					run_scc_norris_count(af, output);
					break;
				default:
					fprintf(stderr, "Problem %s is not supported with algorithm %s.\n", problem, algorithm);
					fclose(output);
					exit(EXIT_FAILURE);
			}
			break;
		case SE_ID:
			{BitSet* ideal = NULL;
			switch (alg) {
				case NEXT_CLOSURE:
					ideal = se_id(af, ee_pr_next_closure);
					break;
				case SCC_NEXT_CLOSURE:
					ideal = se_id(af, ee_pr_scc_next_closure);
					break;
				default:
					print_not_supported(problem, algorithm, output);				
			}
			print_set(ideal, output, "\n");
			free_bitset(ideal);
			free_argumentation_framework(af);}
			break;
		case EE_PR:
			switch (alg) {
				case NEXT_CLOSURE:
					result_list = ee_pr_next_closure(af);
					break;
				case SCC_NEXT_CLOSURE:
					run_scc_next_closure(af, output);
					break;
				default:
					fprintf(stderr, "Problem %s is not supported with algorithm %s.\n", problem, algorithm);
					fclose(output);
					exit(EXIT_FAILURE);
			}
			if (sort_flag) {
				ListNode* node = result_list;
				while (node) {
					// map back the indices if af was sorted before
					BitSet *x = map_indices(node->c, mapping);
					print_set(x, output, "\n");
					node = node->next;
				}
			}
			else {
				print_list(result_list, (void (*)(void *, FILE*, const char*)) print_set, output);
			}
			free_list(result_list, (void (*)(void *)) free_bitset);
			break;
		case SE_PR:
			if (alg == CBO) {
				BitSet* preferred = se_pr_cbo(af);
				print_set(preferred, output, "\n");
				free_bitset(preferred);
			} else {
				print_not_supported(problem, algorithm, output);				
			}
			break;
		case DC_PR:
			if (alg == CBO) {
				BitSet* preferred = dc_pr_cbo(af, --argument);
				if (preferred) {
					print_set(preferred, output, "\n");
					free_bitset(preferred);
				}
			} else {
				print_not_supported(problem, algorithm, output);				
			}
			break;
		case DS_PR:
			if (alg == CBO) {
				BitSet* preferred = ds_pr_cbo(af, --argument);
				if (preferred) {
					print_set(preferred, output, "\n");
					free_bitset(preferred);
				}
			} else {
				print_not_supported(problem, algorithm, output);				
			}
			break;
		case EE_CO:
			switch (alg) {
				case NEXT_CLOSURE:
					result_list = ee_co_next_closure(af);
					break;
				default:
					fprintf(stderr, "Problem %s is not supported with algorithm %s.\n", problem, algorithm);
					fclose(output);
					exit(EXIT_FAILURE);
			}
			if (sort_flag) {
				ListNode* node = result_list;
				while (node) {
					// map back the indices if af was sorted before
					BitSet *x = map_indices(node->c, mapping);
					print_set(x, output, "\n");
					node = node->next;
				}
			}
			else {
				print_list(result_list, (void (*)(void *, FILE*, const char*)) print_set, output);
			}
			free_list(result_list, (void (*)(void *)) free_bitset);
			break;
		case DC_CO:
			// On the command line arguments are named starting from 1. Internally, they start from 0:
			--argument;
			switch (alg) {
				case NEXT_CLOSURE:
					// sort the af in the descending order of victim count
					// (ignores if the af was already sorted)
					// af = sort_af(input_af, VICTIM_COUNT, SORT_DESCENDING);
					// map argument
					// int mapped_argument_index = map_argument(argument);
					// printf("mapped argument: %d\n", mapped_argument_index);
					// result_dc = dc_co_next_closure(af, mapped_argument_index);

					// map back the indices
					// BitSet *x = map_indices(result_dc);
					// print_set(x, output, "\n");
					// free_bitset(x);

					result_dc = dc_co_next_closure_2(af, argument);
					if (result_dc == NULL)
						fprintf(output, "NO\n");
					else {
						print_set(result_dc, output, "\n");
						free_bitset(result_dc);
					}
					break;
				case SUBGRAPH:
					result_dc = dc_co_subgraph_next_closure(af, argument);
					if (result_dc == NULL)
						fprintf(output, "NO\n");
					else {
						print_set(result_dc, output, "\n");
						free_bitset(result_dc);
					}
					break;
				case SUBGRAPH_ADJ:
					result_dc = dc_co_subgraph_next_closure_adj(af, argument);
					if (result_dc == NULL)
						fprintf(output, "NO\n");
					else {
						print_set(result_dc, output, "\n");
						free_bitset(result_dc);
					}
					break;
				default:
					fprintf(stderr, "Problem %s is not supported with algorithm %s.\n", problem, algorithm);
					fclose(output);
					exit(EXIT_FAILURE);
			}
	}

	STOP_TIMER(stop_time);
	printf("Computation time: %.3f milisecs\n", TIME_DIFF(start_time, stop_time) / 1000);

	// close the output file
	fclose(output);

	return(0);
}

