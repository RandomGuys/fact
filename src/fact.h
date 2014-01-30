#ifndef _FACT_H
#define _FACT_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#include <assert.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <gmp.h>
#include <math.h>

typedef struct vec_ {
	mpz_t *el;
	int count;
} vec_t;

void printFinalProduct();

void transformFile(char *file_to_transform);

int buildProductTree (char *moduli_filename);

int buildRemainderTree ();

void iter_threads(int start, int end, void (*func)(int n));

void input_bin_array(vec_t *v, char * filename);

void computeSuperSpeed (char *input);

void init_vec(vec_t *v, int count);

void free_vec(vec_t *v);

double now();

#endif
