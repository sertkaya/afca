#include <assert.h>
#include <stdlib.h>
#include "af.h"
#include "sort.h"
#include "../utils/timer.h"


struct index_value {
	int index; // index of the argument
	double value; // value to use in sorting
};

int cmp_ascending(const void *v1, const void *v2) {
	if ((((struct index_value*) v1)-> value) < (((struct index_value*) v2)-> value))
		return(-1);
	else if ((((struct index_value*) v2)-> value) < (((struct index_value*) v1)-> value))
		return(1);
	else
		return(0);
}
int cmp_descending(const void *v1, const void *v2) {
	if ((((struct index_value*) v1)-> value) < (((struct index_value*) v2)-> value))
		return(1);
	else if ((((struct index_value*) v2)-> value) < (((struct index_value*) v1)-> value))
		return(-1);
	else
		return(0);
}

double victim_count(int victim_count, int attacker_count) {
  return((double) victim_count);
}

double attacker_count(int victim_count, int attacker_count) {
  return((double) attacker_count);
}

double victims_divided_by_attackers(int victim_count, int attacker_count) {
  return(((double) victim_count) / attacker_count);
}

double attackers_divided_by_victims(int victim_count, int attacker_count) {
  return(((double) attacker_count) / victim_count);
}

double victims_minus_attackers(int victim_count, int attacker_count) {
  return(((double) victim_count) - attacker_count);
}

// Sorts the framework according to the given criterion and direction.
// Result is in the sorted_af. Returns the index mapping resulting from sorting
int *sort_af(AF *af, AF *s_af, enum sort_type criterion, enum sort_direction direction) {
	struct timeval start_time, stop_time;
	START_TIMER(start_time);

	struct index_value *index_value_pairs = calloc(af->size, sizeof(struct index_value));
	assert(index_value_pairs != NULL);

    double (*metric_function)(int, int) = NULL;
   	switch (criterion) {
   		case VICTIM_COUNT:
    		metric_function = &victim_count;
    		break;
    	case ATTACKER_COUNT:
    		metric_function = &attacker_count;
        	break;
    	case VICTIMS_DIVIDED_BY_ATTACKERS:
    		metric_function = &victims_divided_by_attackers;
        	break;
        case ATTACKERS_DIVIDED_BY_VICTIMS:
    		metric_function = &attackers_divided_by_victims;
        	break;
        case VICTIMS_MINUS_ATTACKERS:
    		metric_function = &victims_minus_attackers;
            break;
    	default:
        	fprintf(stderr, "Unknown sort type %d. Default is VICTIMS_DIVIDED_BY_ATTACKERS\n", criterion);
    		metric_function = &victims_divided_by_attackers;
	}

	int i,j;
	int victim_count = 0, attacker_count = 0;
	for (i = 0; i < af->size; ++i) {
		index_value_pairs[i].index = i;
		victim_count = count_bits(af->graph[i]);
		attacker_count = 0;
		for (j = 0; j < af->size; ++j)
			if (TEST_BIT(af->graph[j], i))
				++attacker_count;

		index_value_pairs[i].value = metric_function(victim_count, attacker_count);
		// if (victim_count == 0)
		// 	victim_count = 1;
		// index_value_pairs[i].value = ((double) victim_count) / (0.5 * attacker_count);
		// index_value_pairs[i].value = ((double) attacker_count) + 2 *  victim_count;
    }

	// sort the index-value pairs according to value
    if (direction == SORT_DESCENDING)
		qsort(index_value_pairs, af->size, sizeof(index_value_pairs[0]), cmp_descending);
    else
		qsort(index_value_pairs, af->size, sizeof(index_value_pairs[0]), cmp_ascending);

	// create the index mapping
	int *mapping = calloc(af->size, sizeof(int));
	assert(mapping != NULL);

	// fill in the new af sorted and the index mapping
	for (i = 0; i < af->size; ++i) {
		mapping[i] = index_value_pairs[i].index;
		for (j = 0; j < af->size; ++j) {
			if (TEST_BIT(af->graph[index_value_pairs[i].index], index_value_pairs[j].index))
				SET_BIT(s_af->graph[i], j);
		}
	}
	STOP_TIMER(stop_time);
	printf("Sorting time: %.3f milisecs\n", TIME_DIFF(start_time, stop_time) / 1000);

	// print the mapping
	// for (i = 0; i < af->size; ++i)
	// 	printf("%d %d %lf\n", i + 1, index_value_pairs[i].index + 1, index_value_pairs[i].value);

	return(mapping);
}

/*
// map indices from sorted back to original
BitSet *map_indices_back(BitSet *s, int *mapping) {
  int i;
  BitSet* c = create_bitset(s->size);

  for (i = 0; i < s->size; ++i)
    if (TEST_BIT(s, i))
      SET_BIT(c, mapping[i]);

  return(c);
}
*/

int map_argument_back(int argument, int *mapping) {
	return(mapping[argument]);
}

// argument: index of the provided argument (i.e., argument - 1)
// returns index of the mapped argument
int map_argument(int argument, int af_size, int *mapping) {
	int i;
	for (i = 0; i < af_size; ++i) {
		if (mapping[i] == argument)
			return(i);
	}
}