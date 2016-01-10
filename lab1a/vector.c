#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

#include "vector.h"

vector_t simpsh_vector_create()
{
    vector_t vec = (vector_t)malloc(sizeof(struct vector));
    int* storage = (int *)malloc(sizeof(int));
    if (storage == NULL || vec == NULL)
    {
        fprintf(stderr, "Failed to allocate storage\n");
        free(vec);
        free(storage);
        return NULL;
    }
    vec->data = storage;
    vec->cur = 0;
    vec->max = 1;
    return vec;
}

bool simpsh_vector_get_elem(vector_t vec, int index, int* storage)
{
    if (vec == NULL)
        return false;
    if (vec->data == NULL)
        return false;
    if (index < 0 || index >= vec->cur)
        return false;
    if (storage)
        *storage = vec->data[index];
    return true;
}

bool simpsh_vector_insert(vector_t vec, int item)
{
    if (vec == NULL)
        return false;
    if (vec->cur + 1 > vec->max)
    {
        int new_max = vec->max;
        if (new_max > 1)
            new_max *= 1.5;
        else
            new_max += 1;
        int* result = (int *)realloc(vec->data, new_max * sizeof(int));
        if (result == NULL)
            return false;
        vec->data = result;
        vec->max = new_max;
    }
    vec->data[vec->cur] = item;
    vec->cur++;
    return true;
}

int simpsh_vector_get_count(vector_t vec)
{
    if (vec == NULL)
        return -1;
    return vec->cur;
}

void simpsh_vector_delete(vector_t vec)
{
    if (vec == NULL)
        return;
    free(vec->data);
    free(vec);
}
