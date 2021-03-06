/* -*- Mode: C; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: nil -*- */

/**
 * @copyright 2013 Couchbase, Inc.
 *
 * @author Filipe Manana  <filipe@couchbase.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not
 * use this file except in compliance with the License. You may obtain a copy of
 * the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 * License for the specific language governing permissions and limitations under
 * the License.
 **/

#include "view_tests.h"


static int int_cmp_fun(const void *a, const void *b)
{
    return *((int *) a) - *((int *) b);
}


void test_sorted_lists()
{
    int elements[] = { 100, 10, 9, 30, 40, 20, 11, 12 };
    int *sorted_elements;
    int num_elements = sizeof(elements) / sizeof(elements[0]);
    int non_elements[] = { 666, 0, -4, 999, 555 };
    int num_non_elements = sizeof(non_elements) / sizeof(non_elements[0]);
    void *list = sorted_list_create(int_cmp_fun);
    int i;
    void *iterator;

    fprintf(stderr, "Running view sorted_list tests\n");
    sorted_elements = (int *) malloc(sizeof(elements));
    assert(sorted_elements != NULL);
    memcpy(sorted_elements, elements, sizeof(elements));
    qsort(sorted_elements, num_elements, sizeof(sorted_elements[0]), int_cmp_fun);

    assert(list != NULL);
    assert(sorted_list_size(list) == 0);

    for (i = 0; i < num_elements; ++i) {
        int el = elements[i];
        int *copy, *copy2;

        assert(sorted_list_add(list, &el, sizeof(el)) == 0);
        copy = sorted_list_get(list, &el);
        assert(copy != NULL);
        assert(*copy == el);
        assert(copy != &el);

        assert(sorted_list_size(list) == (i + 1));

        /* Insert existing element replaces existing element. */
        assert(sorted_list_add(list, &el, sizeof(el)) == 0);
        copy2 = sorted_list_get(list, &el);
        assert(copy2 != NULL);
        assert(*copy2 == el);
        assert(copy2 != &el);
        assert(copy2 != copy);
    }

    /* Add same elements again. */
    for (i = 0; i < num_elements; ++i) {
        int el = elements[i];

        assert(sorted_list_add(list, &el, sizeof(el)) == 0);
        assert(sorted_list_size(list) == num_elements);
    }

    for (i = num_elements - 1; i >= 0; --i) {
        int el = elements[i];
        int *copy;

        copy = sorted_list_get(list, &el);
        assert(copy != NULL);
        assert(*copy == el);
        assert(copy != &el);
    }

    assert(sorted_list_size(list) == num_elements);

    for (i = 0; i < num_non_elements; ++i) {
        assert(sorted_list_get(list, &non_elements[i]) == NULL);
    }

    assert(sorted_list_size(list) == num_elements);

    for (i = 0; i < num_non_elements; ++i) {
        sorted_list_remove(list, &non_elements[i]);
    }

    assert(sorted_list_size(list) == num_elements);

    for (i = 0; i < num_elements; ++i) {
        int el = elements[i];
        int *copy;

        copy = sorted_list_get(list, &el);
        assert(copy != NULL);
        assert(*copy == el);
        assert(copy != &el);
    }

    assert(sorted_list_size(list) == num_elements);

    iterator = sorted_list_iterator(list);
    assert(iterator != NULL);
    for (i = 0; i < num_elements; ++i) {
        int *e = sorted_list_next(iterator);

        assert(e != NULL);
        assert(*e == sorted_elements[i]);
    }
    assert(sorted_list_next(iterator) == NULL);
    sorted_list_free_iterator(iterator);

    for (i = 0; i < num_elements; ++i) {
        int j;

        sorted_list_remove(list, &elements[i]);

        for (j = i + 1; j < num_elements; ++j) {
            int *copy;

            copy = sorted_list_get(list, &elements[j]);
            assert(copy != NULL);
            assert(*copy == elements[j]);
            assert(copy != &elements[j]);
        }
    }

    assert(sorted_list_size(list) == 0);

    for (i = 0; i < num_elements; ++i) {
        assert(sorted_list_get(list, &elements[i]) == NULL);
    }

    iterator = sorted_list_iterator(list);
    assert(iterator != NULL);
    assert(sorted_list_next(iterator) == NULL);
    sorted_list_free_iterator(iterator);

    sorted_list_free(list);
}
