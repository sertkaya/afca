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

#ifndef ARGUMENT_SET_H
#define ARGUMENT_SET_H

#include <stdio.h>
#include <assert.h>

#include "../af/af.h"
#include "linked_list.h"

struct argument_set {
	bool *vector;
  SIZE_TYPE size;
	ListNode *list;
};
typedef struct argument_set ArgumentSet;

ArgumentSet *new_argument_set(SIZE_TYPE size);

bool argument_set_contains(ARG_TYPE a, ArgumentSet *s);

bool add_to_argument_set(ARG_TYPE a, ArgumentSet *s);

ArgumentSet *duplicate_argument_set(ArgumentSet *s);

bool delete_from_argument_set(ARG_TYPE a, ArgumentSet *s);

void print_argument_set(ArgumentSet *s);

void free_argument_set(ArgumentSet *s);

#endif //ARGUMENT_SET_H
