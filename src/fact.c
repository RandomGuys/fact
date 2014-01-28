#include "fact.h"

#define NTHREADS 4
#define FILE_NAME 25
#define DIR "output_gmp/"
#ifdef mpz_raw_64 // if patched gmp, use large format int i/o
#define __mpz_inp_raw mpz_inp_raw_64
#define __mpz_out_raw mpz_out_raw_64
#else // otherwise use normal i/o...beware 2^31 byte size limit
#define __mpz_inp_raw mpz_inp_raw
#define __mpz_out_raw mpz_out_raw
#endif

typedef struct vec_ {
	mpz_t *el;
	int count;
} vec_t;

int level = 0;

// Initialisation du vecteur
void init_vec(vec_t *v, int count){
    assert(v);
    v->count = count;
    printf("dat cout : %d\n ", count);
    v->el = malloc(count * sizeof(mpz_t));
    assert(v->el);
    for (int i=0; i< v->count; i++)
        mpz_init(v->el[i]);
}

// Libération
void free_vec(vec_t *v){
    assert(v);
    for(int i=0; i < v->count ; i++)
        mpz_clear(v->el[i]);
    free(v->el);
}

// Récupération d'un fichier binaire et stockage dans vecteur
void input_bin_array(vec_t *v, char * filename){
	char tmp_out[100];
	sprintf(tmp_out, "%s%s", DIR, filename);
	FILE *in = fopen(tmp_out, "rb");
	printf("output file : %s\n", tmp_out);
	assert(in);
	int count;
	int ret = fread(&count, sizeof(count), 1, in);
	assert(ret == 1);
	assert(count >= 0);
	init_vec(v, count);
	size_t bytes = 0;
	for (int i=0; i < count; i++)
		bytes += __mpz_inp_raw(v->el[i], in);
	fclose(in);
}

// writes vec_t *v to the named file in binary format
void output_bin_array(vec_t *v, char *filename) {
	fprintf(stderr, "writing %s...", filename);
	char tmp_out[100];
	sprintf(tmp_out, "%s%s", DIR, filename);
	FILE *out = fopen(tmp_out, "wb");
	if (out == NULL) {
		printf("Fichier introuvable\n");
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
		printf("Fichier introuvable\n");
		exit(EXIT_FAILURE);
	}
	int count_l = 0;
	char* fileoutname = (char*) malloc (FILE_NAME * sizeof(char));
	sprintf(fileoutname, "%s%s_gmp", DIR, file_to_transform);

	FILE *out = fopen(fileoutname, "wb");
	mpz_t biginteger;
	mpz_init(biginteger);
	fwrite(&count_l, sizeof(count_l), 1, out);
	while(1) {
		int res = gmp_fscanf(in, "%Zd", biginteger);
		if (res == EOF) {
			break;
		} else if (res != 1) {
			printf("Erreur de lecture\n");
			exit(EXIT_FAILURE);
		} else {
			count_l++;
			mpz_out_raw(out, biginteger);	
		}
	}
	mpz_clear(biginteger);
	fclose(in);
	rewind(out);
	fwrite(&count_l, sizeof(count_l), 1, out);
	fclose(out);
}

// Affiche le produit final en clair
void printFinalProduct() {
	char tmp[100];
	sprintf(tmp, "%sPIntern_%d", DIR, level-1);
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
	fclose(out2);
}

// Fonction pour l'arbre produit (Attention : le fichier en entrée doit être un fichier clair)
int buildProductTree (char *moduli_filename) {
	// Ouverture du fichier
	FILE* moduli = fopen(moduli_filename, "r");
	if (moduli == NULL) {
		printf("Fichier introuvable\n");
		exit(EXIT_FAILURE);
	}
	vec_t v;
	transformFile(moduli_filename);
	sprintf(moduli_filename, "%s_gmp", moduli_filename);
	input_bin_array(&v, moduli_filename);
	
	printf("Starting\n");
	while (v.count > 1) {
		
		fprintf(stderr, "level %d\n", level);
		vec_t w;
		init_vec(&w,(v.count+1)/2);

		void mul_job(int i) {
			mpz_mul(w.el[i], v.el[2*i], v.el[2*i+1]);
		}
		printf("Starting\n");
		iter_threads(0, v.count/2, mul_job);
		if (v.count & 1)
			mpz_set(w.el[v.count/2], v.el[v.count-1]); 

		char name[255];
		snprintf(name, sizeof(name)-1, "PIntern_%d", level);
		output_bin_array(&w, name);

		//free_vec(&v);
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

int buildRemainderTree (int level) {
	return 0;
}