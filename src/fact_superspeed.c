#include "fact.h"

#define NTHREADS 100
#define MODE_PRODUCT 1
#define MODE_REMAINDER -1

typedef struct tree_ {
	int levels_count;
	vec_t *levels;
} tree_t;

tree_t product_tree = {0};

void iter_threads_superspeed (int mode, void (*func)(int n, int m)) {
	int n = 0;
	int m = 1;
	if (mode == MODE_REMAINDER) {
		m = product_tree.levels_count;
	}
	pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

	void *thread_body(void *ptr) {
	while (1) {
		pthread_mutex_lock(&mutex);
		if (n >= product_tree.levels[m].count && m == product_tree.levels_count - 1) {
			pthread_mutex_unlock(&mutex);
			break;
		}
		int i = n++;
		int j = m;
		if (i >= product_tree.levels[j].count && m < product_tree.levels_count - 1) {			
			m++;
			n = 0;
			pthread_mutex_unlock(&mutex);
			continue;
		}
		
		pthread_mutex_unlock(&mutex);
		if (j == product_tree.levels_count) {
			break;
		}
		func(i, j);
	}

	return NULL;
	}

	pthread_t thread_id[NTHREADS];
	for (int i=0; i < NTHREADS; i++)
		pthread_create(&thread_id[i], NULL, thread_body, NULL);

	for (int i=0; i < NTHREADS; i++)
		pthread_join(thread_id[i], NULL);
}

void computeSuperSpeed (char *input) {
	vec_t v = {0};
	// Transform clear text input into gmp binary
	transformFile(input);
	char *moduli_filename = "PInterm1";
	input_bin_array(&v, moduli_filename);
	
	int count = v.count;
	int levels_count = (int) ceil (log (count) / log (2)) + 1;
	product_tree.levels_count = levels_count;
	
	printf ("Allocating tree space...\n");
	product_tree.levels = (vec_t *) malloc (levels_count * sizeof (vec_t));
	product_tree.levels[0] = v;
	int count_i = count / 2;
	for (int i = 1; i < levels_count; i++) {
		init_vec (product_tree.levels + i, count_i);
		product_tree.levels[i].count = count_i;
		//~ printf ("count_%d = %d\n", i, product_tree.levels[i].count);
		count_i = count_i / 2;
	}
	printf ("Done.\n\n");
	
	printf("Building product tree...\n");
	double start = now ();
	for (int i = 1; i < levels_count; i++) {
		vec_t *w = product_tree.levels + i;
		vec_t *v2 = product_tree.levels + i - 1;
		void mul_job(int i) {
			mpz_mul(w->el[i], v2->el[2*i], v2->el[2*i+1]);
		}
		iter_threads(0, v2->count/2, mul_job);

		if (v2->count & 1)
			mpz_set(w->el[v2->count/2], v.el[v2->count-1]); 

	}
	printf ("Done in %0.10fs\n", now () - start);
	
	
	// REMAINDER TREE
	int secL;
	//int j = 0;
	int sizeP; // à modifier, ceci peut se récupérer dans le fichier
	
	// On charge le premier élément P dans notre liste de previous
	sizeP = 1;
	vec_t *modPre = product_tree.levels + product_tree.levels_count - 1;
	
	vec_t modCur,*prodL,gcd;
	
	secL = product_tree.levels_count;
	
	printf("Building remainder tree...\n");
	start = now ();
	do {
		// Création du vecteur current contenant les modulos calculés
		sizeP *= 2; 
		init_vec(&modCur,sizeP);	
	
		// Création du vecteur prod contenant la ligne des produits correspodant	
		prodL = product_tree.levels + secL - 2;
		void mul_job(int j){			
			mpz_pow_ui(prodL->el[j],prodL->el[j],2); // v= v^2
			mpz_mod(modCur.el[j],modPre->el[j/2],prodL->el[j]); // sol = produit mod v		
		}
		iter_threads(0, prodL->count, mul_job);
		
		// On assigne tous les éléments de modCur à modPre (on vide modPre avant)
		*modPre = modCur;
		free_vec (prodL);	
	} while (--secL > 1);
	
	
	start = now ();
	// ETAPE N : div, puis gcd
	int j=0;
	init_vec(&gcd,modCur.count);
	
	input_bin_array (&v, moduli_filename);
	void mul_job(int j){
		mpz_divexact(modCur.el[j],modCur.el[j],v.el[j]); // sol = sol / item 
		mpz_gcd (gcd.el[j],modCur.el[j],v.el[j]); // gcd = pgcd(sol,item)
	}
	iter_threads(0, v.count, mul_job);
	printf ("Done in %0.10fs\n", now () - start);
	
	printf ("Sorting moduli...\n");
	start = now ();
	vec_t vuln_moduli, prime_gcds;
	init_vec (&vuln_moduli, v.count);
	init_vec (&prime_gcds, v.count);
	int size = 0, size_prime = 0;
	for(j = 0; j < v.count; j++) {
		//~ gmp_printf("j= %d  : PGCD=%Zd \n\n",j,gcd.el[j]);
		if (mpz_cmp_ui (gcd.el[j], 1) != 0) {
			mpz_set (vuln_moduli.el[size++], v.el[j]);
			if (mpz_probab_prime_p (gcd.el[j], 25) != 0) {
				int exist = 0;
				for (int l = 0; l < size_prime && !exist; l++) {
					exist = !mpz_cmp (prime_gcds.el[l], gcd.el[j]);
				}
				if (!exist) {
					mpz_set (prime_gcds.el[size_prime++], gcd.el[j]);
				}
			}
		}
	}
	vuln_moduli.count = size;
	prime_gcds.count = size_prime;
	
	printf ("Done in %0.10fs\n", now () - start);
	
	//~ for (int i = 0; i < vuln_moduli.count; i++) {
		//~ gmp_printf ("%Zd\t", vuln_moduli.el[i]);
	//~ }
	//~ printf ("\n\n");
	//~ 
	//~ for (int i = 0; i < prime_gcds.count; i++) {
		//~ gmp_printf ("%Zd\t", prime_gcds.el[i]);
	//~ }
	//~ printf ("\n\n");
	
	FILE *result = fopen ("output", "w");
	printf ("Writing output...\n");
	for (int j = 0; j < prime_gcds.count; j++) {
		gmp_fprintf (result, "%Zd, ", prime_gcds.el[j]);
		for (int i = 0; i < vuln_moduli.count; i++) {
			mpz_t t;
			mpz_init (t);
			mpz_gcd (t, vuln_moduli.el[i], prime_gcds.el[j]);
			if (mpz_cmp_ui (t, 1) != 0) {
				gmp_fprintf (result, "%Zd, ", vuln_moduli.el[i]);
			}
			mpz_clear (t);
		}
		fprintf (result, "\n");
	}
	printf ("Done in %0.10fs\n", now () - start);
	fclose (result);
	
	free_vec (&gcd);
	free_vec (&v);	
}
