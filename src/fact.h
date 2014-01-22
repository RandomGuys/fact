#ifndef _FACT_H
#define _FACT_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <pthread.h>
#include <gmp.h>

int buildProductTree (char *moduli_filename);

int buildRemainderTree (int level);

#endif
