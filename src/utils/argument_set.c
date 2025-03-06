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

#include "argument_set.h"


ArgumentSet *new_argument_set(SIZE_TYPE size) {
	ArgumentSet *new_set = calloc(1, sizeof(ArgumentSet));
    assert(new_set != NULL);

    new_set->vector = calloc(size, sizeof(bool));
    assert(new_set->vector != NULL);

    new_set->size = size;
    new_set->list = NULL;

    return(new_set);
}

bool argument_set_contains(ARG_TYPE a, ArgumentSet *s) {
  return(s->vector[a]);
}

bool add_to_argument_set(ARG_TYPE a, ArgumentSet *s) {
  if (s->vector[a])
    return(false);

  s->vector[a] = true;
  s->list = insert_list_int(a, s->list);
  return(true);
}

ArgumentSet *duplicate_argument_set(ArgumentSet *s) {
  ArgumentSet *copy = new_argument_set(s->size);
  ListNode *tmp = s->list;
  while (tmp) {
    add_to_argument_set(tmp->e->n, copy);
    tmp = tmp->next;
  }
  return(copy);
}

bool delete_from_argument_set(ARG_TYPE a, ArgumentSet *s) {
  if (!s->vector[a])
    return(false);

  s->vector[a] = false;
  ListNode *previous = NULL;
  ListNode *current = s->list;

  // if a is the first element
  if (current->e->n == a) {
    s->list = current->next;
    free(current->e);
    free(current);
    return(true);
  }
  // otherwise
  while (current) {
    if (current->e->n == a) {
      previous->next = current->next;
      free(current->e);
      free(current);
      break;
    }
    previous = current;
    current = current->next;
  }

  return(true);
}

void print_argument_set(ArgumentSet *s) {
  ListNode *tmp = s->list;
  while (tmp) {
    printf("%d ", tmp->e->n);
    tmp = tmp->next;
  }
  printf("\n");
}

void free_list_element(ListElement* e) {
  free(e);
}

void free_argument_set(ArgumentSet *s) {
  free(s->vector);
  free_list(s->list, free_list_element);
  free(s);
}
