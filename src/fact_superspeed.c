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
	printf ("BEGIN computeSuperSpeed (%s)\n", input);
	vec_t v = {0};
	// Transform clear text input into gmp binary
	transformFile(input);
	char *moduli_filename = "PInterm1";
	input_bin_array(&v, moduli_filename);
	
	int count = v.count;
	int levels_count = (int) ceil (log (count) / log (2)) + 1;
	product_tree.levels_count = levels_count;
	printf ("Levels count = %d\n", levels_count);
	
	printf ("Allocating tree space...\n");
	product_tree.levels = (vec_t *) malloc (levels_count * sizeof (vec_t));
	product_tree.levels[0] = v;
	int count_i = count / 2;
	for (int i = 1; i < levels_count; i++) {
		init_vec (product_tree.levels + i, count_i);
		product_tree.levels[i].count = count_i;
		printf ("count_%d = %d\n", i, product_tree.levels[i].count);
		count_i = count_i / 2;
	}
	printf ("Done.\n\n");
	
	printf("Starting...\n");
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
	gmp_printf ("Done P = %Zd in %0.10fs\n", product_tree.levels[levels_count -1].el[0], now () - start);
	//~ printf ("Done in %0.10fs\n", now () - start);
	
	
	// REMAINDER TREE
	int secL;
	int j = 0;
	int k, sizeP; // à modifier, ceci peut se récupérer dans le fichier
	
	// On charge le premier élément P dans notre liste de previous
	sizeP = 1;
	vec_t *modPre = product_tree.levels + product_tree.levels_count - 1;
	
	vec_t modCur,*prodL,lastLineSquared,gcd;
	
	secL = product_tree.levels_count;
	
	printf("LEVEL  : %d DEBUT DE LA DESCENTE\n", levels_count);
	
	while (secL > 0){
		// Création du vecteur current contenant les modulos calculés
		sizeP *= 2; 
		printf("sizeP : %d\n",sizeP);
		init_vec(&modCur,sizeP);	

	
		// Création du vecteur prod contenant la ligne des produits correspodant	
		prodL = product_tree.levels + secL - 1;
		j=0;		
		k=0;
		
		void mul_job(int j){
			// Test pour savoir sur quel P ou resultat de moduli on travaille
			if(j%2==0&&j!=0){
				k=k+1;
			}
			
			gmp_printf("V  : %Zd    j=%d \n",prodL->el[j],j);
			gmp_printf("P  : %Zd  k=%d \n",modPre->el[k],k);
				// Si le prodL vaut 1 alors on passe à la suivante ou mod précédent = 0
				if (mpz_cmp_ui(prodL->el[j],1)==0||mpz_cmp_ui(modPre->el[k],0)==0){
					mpz_set_ui(modCur.el[j],1);
				}
				
				// Sinon
				else {
					// Si c'est la dernière étape (Ni)
					if(sizeP >= levels_count - 1) {
						printf("DERNIER TOUR!");
						init_vec(&lastLineSquared,prodL->count);	
						mpz_pow_ui(lastLineSquared.el[j],prodL->el[j],2);
						mpz_mod(modCur.el[j],modPre->el[k],lastLineSquared.el[j]); // sol = produit mod v			
					}
				
					else{
						mpz_pow_ui(prodL->el[j],prodL->el[j],2); // v= v^2
						mpz_mod(modCur.el[j],modPre->el[k],prodL->el[j]); // sol = produit mod v			
					}
				}
		
			gmp_printf("modCur : %Zd\n", modCur.el[j]); 			
			j=j+1;			
		}
		iter_threads(0, prodL->count, mul_job);
		
		
		
		// On assigne tous les éléments de modCur à modPre (on vide modPre avant)
		modPre->count = modCur.count;
		modPre->el = modCur.el;			
		gmp_printf("P après libération : %Zd size %d \n",modPre->el[0],modPre->count);
		secL=secL-1;
	}
	
	// ETAPE N : div, puis gcd
	j=0;
	init_vec(&gcd,modCur.count);
	
	void mul_job(int j){
		mpz_divexact(modCur.el[j],modCur.el[j],prodL->el[j]); // sol = sol / item 
		gmp_printf("j= %d modCur.el[j] = %Zd\n",j,modCur.el[j]);
		if(mpz_cmp_ui(modCur.el[j],0)==0 ){
			mpz_set_ui(gcd.el[j],1);
		}
		else{
			mpz_gcd (gcd.el[j],modCur.el[j],prodL->el[j]); // gcd = pgcd(sol,item)
		}
	}
	iter_threads(0, prodL->count, mul_job);
	
	printf("\n\n");
	for(j=0;j<prodL->count;j++)
		gmp_printf("j= %d  : PGCD=%Zd \n\n",j,gcd.el[j]);
	
	free_vec (&gcd);
	free_vec (&lastLineSquared);
	
	
	printf ("Freeing tree space...\n");
	for (int i = 0; i < levels_count; i++) {
		free_vec (product_tree.levels + i);
	}
	free (product_tree.levels);
	printf ("Done.\n\n");

	printf ("END computeSuperSpeed\n");
	
}
