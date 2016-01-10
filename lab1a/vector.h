#ifndef VECTOR_H
#define VECTOR_H

typedef struct vector
{
    int* data;
    int cur;
    int max;
} *vector_t;

vector_t simpsh_vector_create();
bool simpsh_vector_get_elem(vector_t vec, int index, int* storage);
bool simpsh_vector_insert(vector_t vec, int item);
int simpsh_vector_get_count(vector_t vec);
void simpsh_vector_delete(vector_t vec);

#endif
