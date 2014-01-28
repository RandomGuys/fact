#include "fact.h"

typedef struct tree_ {
	int levels_count;
	vec_t *levels;
} tree_t;

void computeSuperSpeed (char *input) {
	printf ("BEGIN computeSuperSpeed (%s)\n", input);
	tree_t product_tree = {0};
	vec_t v = {0};
	// Transform clear text input into gmp binary
	transformFile(input);
	char *moduli_filename = "PInterm1";
	input_bin_array(&v, moduli_filename);
	
	int count = v.count;
	int levels_count = (int) ceil (log (count) / log (2)) + 1;
	printf ("Levels count = %d\n", levels_count);
	
	printf ("Allocating tree space...\n");
	product_tree.levels = (vec_t *) malloc (levels_count * sizeof (vec_t));
	product_tree.levels[0] = v;
	for (int i = 1; i < levels_count; i++) {
		int count_i = count = 2 * count / (i + 1);
		init_vec (product_tree.levels + i, count_i);
		product_tree.levels[i].count = count_i;
	}
	printf ("Done.\n\n");
	
	printf("Starting...\n");
	for (int i = 1; i < levels_count; i++) {
		vec_t w;
		v = product_tree.levels[i - 1];
		init_vec(&w,(v.count+1)/2);
		void mul_job(int i) {
			mpz_mul(w.el[i], v.el[2*i], v.el[2*i+1]);
		}
		iter_threads(0, v.count/2, mul_job);
		if (v.count & 1)
			mpz_set(w.el[v.count/2], v.el[v.count-1]); 

		product_tree.levels[i] = w;
	}
	gmp_printf ("Done. P = %Zd in\n", product_tree.levels[levels_count - 1].el[0]);
	
	printf ("Freeing tree space...\n");
	for (int i = 0; i < levels_count; i++) {
		free_vec (product_tree.levels + i);
	}
	free (product_tree.levels);
	printf ("Done.\n\n");

	printf ("END computeSuperSpeed\n");
}
