#include "fact.h"

#define NTHREADS 100
#define MODE_PRODUCT 1
#define MODE_REMAINDER -1

typedef struct tree_ {
	int levels_count;
	vec_t *levels;
} tree_t;

tree_t product_tree = {0};

extern int input_type;

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
		printf("Computing level %d...", secL);
		double start_level = now();
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
		printf (" done in %0.10fs\n", now() - start_level);
	} while (--secL > 1);
	

	// ETAPE N : div, puis gcd
	init_vec(&gcd,modCur.count);
	
	input_bin_array (&v, moduli_filename);
	void mul_job(int j){
		mpz_divexact(modCur.el[j],modCur.el[j],v.el[j]); // sol = sol / item 
		mpz_gcd (gcd.el[j],modCur.el[j],v.el[j]); // gcd = pgcd(sol,item)
	}
	iter_threads(0, v.count, mul_job);
	printf ("Done in %0.10fs\n", now () - start);
	
	
	vec_t potentielVuln, premiers;
	init_vec (&potentielVuln, gcd.count);
	init_vec (&premiers, gcd.count);
	mpz_t q;
	mpz_init (q);
	
	int pvuln = 0, ppremier = 0;
	printf ("Identification des premiers et potentiels moduli vulnérables...\n");
	start = now ();
	for (int i = 0; i < gcd.count; i++) {
		if (mpz_cmp_ui (gcd.el[i], 1) != 0) {
			mpz_set (potentielVuln.el[pvuln++], v.el[i]);
			if (mpz_probab_prime_p (gcd.el[i], 25) != 0) {
				mpz_set (premiers.el[ppremier++], gcd.el[i]);
			}
		}
	}
	printf ("Done in %0.10fs\n", now () - start);
	potentielVuln.count = pvuln;
	premiers.count = ppremier;
	
	printf ("Identification des moduli vulnérables...\n");
	FILE *result = fopen ("moduli_p_q", "w");
	start = now ();
	for (int i = 0; i < potentielVuln.count; i++) {
		for (int j = 0; j < premiers.count; j++) {
			if (mpz_divisible_p (potentielVuln.el[i], premiers.el[j]) != 0) {
				mpz_divexact (q, potentielVuln.el[i], premiers.el[j]);
				if (mpz_probab_prime_p (q, 25) != 0) {
					if (input_type == INPUT_HEXA) 
						gmp_fprintf (result, "%ZX, %ZX, %ZX\n", potentielVuln.el[i], premiers.el[j], q);
					else
						gmp_fprintf (result, "%Zd, %Zd, %Zd\n", potentielVuln.el[i], premiers.el[j], q);
				}
			}
		}
	}
	printf ("Done in %0.10fs\n", now () - start);
	free_vec (&potentielVuln);
	free_vec (&premiers);
	fclose (result);

	free_vec (&gcd);
	free_vec (&v);	
}
