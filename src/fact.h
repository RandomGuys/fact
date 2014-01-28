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

void printFinalProduct();

void transformFile(char *file_to_transform);

int buildProductTree (char *moduli_filename);

int buildRemainderTree ();

void iter_threads(int start, int end, void (*func)(int n));

#endif
