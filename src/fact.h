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
#include <getopt.h>

#define FILE_NAME 25
#define DIR "output_gmp/"
#define MODE_FILES 0
#define MODE_FULLRAM 1
#define INPUT_DECIMAL 0
#define INPUT_HEXA 1

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
