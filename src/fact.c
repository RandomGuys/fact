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

int level = 2;

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
    for(int i=0; i < v->count ; i++)
        mpz_clear(v->el[i]);
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
		printf("transformFile : Fichier introuvable\n");
		exit(EXIT_FAILURE);
	}
	int count_l = 0;
	char* fileoutname = (char*) malloc (FILE_NAME * sizeof(char));
	sprintf(fileoutname, "%sPInterm1", DIR);

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
		snprintf(name, sizeof(name)-1, "PInterm%d", level);
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

int buildRemainderTree () {
	int secL;
	int j=0;
	int k,sizeP; // à modifier, ceci peut se récupérer dans le fichier
	
	mpz_t p,v,sol,v2,temp;

	
	mpz_init(p);
	mpz_init(v);
	mpz_init(sol);
	mpz_init(v2);
	mpz_init(temp);
//	mpz_init(gcd);
	
	// On charge le premier élément P dans notre liste de previous
	sizeP=1;
	vec_t modPre;
	init_vec(&modPre,sizeP);	
	input_bin_array(&modPre,"PFinal");

	

	vec_t modCur,prodL,lastLineSquared,gcd;
	
	
	// ETAPE 1 : calcul des modulos
	// nombre d'éléments de notre array (P puis moduli calculés)
	secL=level;
	char str[15]="";
	
	
	printf("LEVEL  : %d DEBUT DE LA DESCENTE\n", level);
	while (sizeP < 2*level && secL > 0){
		// Création du vecteur current contenant les modulos calculés
		sizeP=sizeP*2; 
		printf("sizeP : %d\n",sizeP);
		init_vec(&modCur,sizeP);	

	
		// Création du vecteur prod contenant la ligne des produits correspodant
		init_vec(&prodL,sizeP);	
		sprintf(str, "PInterm%d",secL);
		printf("S :: %s\n",str);
		input_bin_array(&prodL,str);	
		printf("str : %s\n", str);
		j=0;		
		k=0;
		
/*		void mul_job(int j){*/
		while(j < prodL.count){	
			// Test pour savoir sur quel P ou resultat de moduli on travaille
			if(j%2==0&&j!=0){
				k=k+1;
			}
			
			gmp_printf("V  : %Zd    j=%d \n",prodL.el[j],j);
			gmp_printf("P  : %Zd  k=%d \n",modPre.el[k],k);
				// Si le prodL vaut 1 alors on passe à la suivante ou mod précédent = 0
				if (mpz_cmp_ui(prodL.el[j],1)==0||mpz_cmp_ui(modPre.el[k],0)==0){
					mpz_set_ui(modCur.el[j],1);
				}
				
				// Sinon
				else {
					// Si c'est la dernière étape (Ni)
					if(sizeP>=level*2) {
						printf("DERNIER TOUR!");
						init_vec(&lastLineSquared,prodL.count);	
						mpz_pow_ui(lastLineSquared.el[j],prodL.el[j],2);
						mpz_mod(modCur.el[j],modPre.el[k],lastLineSquared.el[j]); // sol = produit mod v			
					}
				
					else{
						mpz_pow_ui(prodL.el[j],prodL.el[j],2); // v= v^2
						mpz_mod(modCur.el[j],modPre.el[k],prodL.el[j]); // sol = produit mod v			
					}
				}
		
			gmp_printf("modCur : %Zd\n", modCur.el[j]); 			
			j=j+1;			
		}
		//iter_threads(0, prodL.count, mul_job);
		
		
		
		// On assigne tous les éléments de modCur à modPre (on vide modPre avant)
		free_vec(&modPre);
		modPre.count = modCur.count;
		modPre.el = modCur.el;			
		gmp_printf("P après libération : %Zd size %d \n",modPre.el[0],modPre.count);
		secL=secL-1;
	}
	

	
	// ETAPE N : div, puis gcd
	j=0;
	init_vec(&gcd,modCur.count);
	
	void mul_job(int j){
		mpz_divexact(modCur.el[j],modCur.el[j],prodL.el[j]); // sol = sol / item 
		gmp_printf("j= %d modCur.el[j] = %Zd\n",j,modCur.el[j]);
		if(mpz_cmp_ui(modCur.el[j],0)==0 ){
			mpz_set_ui(gcd.el[j],1);
		}
		else{
			mpz_gcd (gcd.el[j],modCur.el[j],prodL.el[j]); // gcd = pgcd(sol,item)
		}
	}
	iter_threads(0, prodL.count, mul_job);
	
	printf("\n\n");
	for(j=0;j<prodL.count;j++)
		gmp_printf("j= %d  : PGCD=%Zd \n\n",j,gcd.el[j]);
	

	return 0;
}
