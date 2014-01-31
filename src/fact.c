#include "fact.h"

#define NTHREADS 2
#define FILE_NAME 25
#define DIR "output_gmp/"
#ifdef mpz_raw_64 // if patched gmp, use large format int i/o
#define __mpz_inp_raw mpz_inp_raw_64
#define __mpz_out_raw mpz_out_raw_64
#else // otherwise use normal i/o...beware 2^31 byte size limit
#define __mpz_inp_raw mpz_inp_raw
#define __mpz_out_raw mpz_out_raw
#endif

int level = 2;

double now()
{
    struct timeval t;
    gettimeofday(&t, NULL);
    return (double) t.tv_sec + (double) t.tv_usec / 1000000.;
}

// Initialisation du vecteur
void init_vec(vec_t *v, int count){
    assert(v);
    v->count = count;
    v->el = malloc(count * sizeof(mpz_t));
    assert(v->el);
    for (int i=0; i< v->count; i++)
        mpz_init(v->el[i]);
}

// Libération
void free_vec(vec_t *v){
    assert(v);
    for(int i=0; i < v->count ; i++) {
        mpz_clear(v->el[i]);
	}
    free(v->el);
}

// Récupération d'un fichier binaire et stockage dans vecteur
void input_bin_array(vec_t *v, char * filename){
	char tmp_out[100];
	sprintf(tmp_out, "%s%s", DIR, filename);
	FILE *in = fopen(tmp_out, "rb");
	printf("INPUT file : %s\n", tmp_out);
	assert(in);
	int count;
	int ret = fread(&count, sizeof(count), 1, in);
	assert(ret == 1);
	assert(count >= 0);
	init_vec(v, count);
	size_t bytes = 0;
	
	for (int i=0; i < count; i++){
		bytes += __mpz_inp_raw(v->el[i], in);
		
	}
		
		
	fclose(in);
}

// writes vec_t *v to the named file in binary format
void output_bin_array(vec_t *v, char *filename) {
	fprintf(stderr, "writing %s...", filename);
	char tmp_out[100];
	sprintf(tmp_out, "%s%s", DIR, filename);
	FILE *out = fopen(tmp_out, "wb");
	if (out == NULL) {
		printf("output_bin_array  : Fichier introuvable\n");
		exit(EXIT_FAILURE);
	}
	fwrite(&v->count, sizeof(v->count), 1, out);
	size_t bytes = 0;
	for (int i=0; i < v->count; i++)
		bytes += __mpz_out_raw(out, v->el[i]);
	fclose(out);
}

// Fonction transformant un fichier texte clair en texte binaire
void transformFile(char *file_to_transform) {
	FILE *in = fopen(file_to_transform, "r");
	if (in == NULL) {
		printf("transformFile : Fichier introuvable : %s\n", file_to_transform);
		exit(EXIT_FAILURE);
	}
	int count_l = 0;
	char* fileoutname = (char*) malloc (FILE_NAME * sizeof(char));
	sprintf(fileoutname, "%sPInterm1", DIR);

	FILE *out = fopen(fileoutname, "wb");
	mpz_t biginteger;
	mpz_init(biginteger);
	fwrite(&count_l, sizeof(count_l), 1, out);
	int l = 0;
	while(1) {
		int res = gmp_fscanf(in, "%Zx", biginteger);
		if (res == EOF) {
			break;
		} else if (res != 1) {
			printf("Erreur de lecture à la ligne %d\n", l);
			exit(EXIT_FAILURE);
		} else {
			count_l++;
			mpz_out_raw(out, biginteger);	
		}
		l++;
	}
	int nearest_exp = (int) (ceil(log(count_l)/log(2)));
	int count_l2 = (int) pow (2., (double) nearest_exp);
	printf ("Padding with %d ones...\n", count_l2 - count_l);
	mpz_set_ui (biginteger, 1);
	for (int i = 0; i < count_l2 - count_l; i++) {
		mpz_out_raw (out, biginteger);
	}
	mpz_clear(biginteger);
	fclose(in);
	rewind(out);
	fwrite(&count_l2, sizeof(count_l2), 1, out);
	fclose(out);
}

// Affiche le produit final en clair
void printFinalProduct() {
	char tmp[100];
	level = level - 1;		// Final
	sprintf(tmp, "%sPInterm%d", DIR, level);
	level = level - 1;		// Last intermediate
	char final[50];
	sprintf(final, "mv %s %sPFinal", tmp, DIR);
	printf("Printing : %s\n", tmp);
	FILE* out2 = fopen(tmp, "rb");
	int count;
	int ret = fread(&count, sizeof(count), 1, out2);
	assert(ret == 1);
	mpz_t bigintegergmp;
	mpz_init(bigintegergmp);

	size_t bytes = 0;
	bytes = __mpz_inp_raw(bigintegergmp, out2);
	
	gmp_printf("Final %zu: %Zd\n", bytes, bigintegergmp);
	mpz_clear(bigintegergmp);
	printf("Final Level %d\n", level);
	fclose(out2);
	system(final);
}

// Fonction pour l'arbre produit (Attention : le fichier en entrée doit être un fichier clair)
int buildProductTree (char *moduli_filename) {
	// Ouverture du fichier
	FILE* moduli = fopen(moduli_filename, "r");
	if (moduli == NULL) {
		printf("buildProducTree: Fichier introuvable\n");
		exit(EXIT_FAILURE);
	}
	vec_t v;
	transformFile(moduli_filename);
	sprintf(moduli_filename, "PInterm1");
	input_bin_array(&v, moduli_filename);
	
	printf("Starting\n");


	while (v.count > 1) {
		vec_t w;
		fprintf(stderr, "level %d\n", level);
		init_vec(&w,(v.count+1)/2);

		void mul_job(int i) {
			mpz_mul(w.el[i], v.el[2*i], v.el[2*i+1]);
		}
		printf("Starting\n");
		iter_threads(0, v.count/2, mul_job);
		if (v.count & 1)
			mpz_set(w.el[v.count/2], v.el[v.count-1]); 

		char name[255];
		snprintf(name, sizeof(name)-1, "PInterm%d", level);
		output_bin_array(&w, name);

		free_vec(&v);
		v = w;
		level++;
	}


	free_vec(&v);	
	return 0;
}

void iter_threads(int start, int end, void (*func)(int n)) {
	int n = start;
	pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

	void *thread_body(void *ptr) {
	while (1) {
		pthread_mutex_lock(&mutex);
		int i = n++;
		pthread_mutex_unlock(&mutex);
		if (i >= end)
			break;
		func(i);
	}

	return NULL;
	}

	pthread_t thread_id[NTHREADS];
	for (int i=0; i < NTHREADS; i++)
		pthread_create(&thread_id[i], NULL, thread_body, NULL);

	for (int i=0; i < NTHREADS; i++)
		pthread_join(thread_id[i], NULL);
}

int buildRemainderTree () {
	int secL;
	int j=0,sizeP=1; // à modifier, ceci peut se récupérer dans le fichier
	
	// On charge le premier élément P dans notre liste de previous


	vec_t modCur,gcd,v;
	vec_t *modPre,*prodL;
	init_vec(modPre,sizeP);	
	input_bin_array(modPre,"PFinal");

	printf("LEVEL  : %d DEBUT DE LA DESCENTE\n", level);
	
	// ETAPE 1 : calcul des modulos
	secL=level;
	char str[15]="";
	do {
		// Création du vecteur current contenant les modulos calculés
		sizeP *= 2; 
		init_vec(&modCur,sizeP);	
	
		// Création du vecteur prod contenant la ligne des produits correspodant	
		init_vec(&prodL,sizeP);	
		sprintf(str, "PInterm%d",secL);
		input_bin_array(&prodL,str);	

		void mul_job(int j){			
			mpz_pow_ui(prodL->el[j],prodL->el[j],2); // v= v^2
			mpz_mod(modCur.el[j],modPre->el[j/2],prodL->el[j]); // sol = produit mod v			
		}
		iter_threads(0, prodL->count, mul_job);
		
		// On assigne tous les éléments de modCur à modPre (on vide modPre avant)
		*modPre = modCur;
	} while (--secL > 1);
	
	// ETAPE N : div, puis gcd
	init_vec(&gcd,modCur.count);
	input_bin_array (&v,"PInterm1");
	void mul_job(int j){
		mpz_divexact(modCur.el[j],modCur.el[j],v.el[j]); // sol = sol / item 
		gmp_printf("j= %d modCur.el[j] = %Zd\n",j,modCur.el[j]);
		mpz_gcd (gcd.el[j],modCur.el[j],v.el[j]); // gcd = pgcd(sol,item)
	}
	iter_threads(0, v.count, mul_job);
	
	printf("\n\n");
	
	printf ("Sorting moduli\n");

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
	
	
	//~ for (int i = 0; i < vuln_moduli.count; i++) {
		//~ gmp_printf ("%Zd\t", vuln_moduli.el[i]);
	//~ }
	//~ printf ("\n\n");
	//~ 
	//~ for (int i = 0; i < prime_gcds.count; i++) {
		//~ gmp_printf ("%Zd\t", prime_gcds.el[i]);
	//~ }
	//~ printf ("\n\n");
	
	vec_t *moduli_by_prime;
	FILE *result = fopen ("output", "w");
	printf ("Writing output\n");
	moduli_by_prime = (vec_t *) malloc (prime_gcds.count * sizeof (vec_t));
	for (int j = 0; j < prime_gcds.count; j++) {
		init_vec (moduli_by_prime + j, vuln_moduli.count + 1);
		moduli_by_prime[j].count = 1;
		mpz_set (moduli_by_prime[j].el[0], prime_gcds.el[j]);
		gmp_fprintf (result, "%Zd, ", moduli_by_prime[j].el[0]);
		for (int i = 0; i < vuln_moduli.count; i++) {
			mpz_t t;
			mpz_init (t);
			mpz_gcd (t, vuln_moduli.el[i], prime_gcds.el[j]);
			if (mpz_cmp_ui (t, 1) != 0) {
				gmp_fprintf (result, "%Zd, ", vuln_moduli.el[i]);
				mpz_set (moduli_by_prime[j].el[moduli_by_prime[j].count], vuln_moduli.el[i]);
				moduli_by_prime[j].count++;
			}
			mpz_clear (t);
		}
		fprintf (result, "\n");
	}
	fclose (result);
	
	for (int j = 0; j < prime_gcds.count; j++) {
		free_vec (moduli_by_prime + j);
	}
	free (moduli_by_prime);
	
	free_vec (&gcd);
	free_vec (&v);

	printf ("END computeSuperSpeed\n");


	return 0;
}
