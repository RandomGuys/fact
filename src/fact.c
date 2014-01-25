#include "fact.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

// Fonction comptant le nombre de lignes d'un fichier
int countLines (FILE *fp) {
	if (fp == NULL) {
		printf("Fichier incorrect\n");
		exit(EXIT_FAILURE);
	}
	int n = 0, c;
	printf("Counting Lines...");
	while ((c = fgetc(fp)) != EOF) {
		if (c == '\n') {
			n++;
		}
	}
	printf("Done! -- %d \n", n);
	fclose(fp);
	return n;
}

// Fonction transformant un fichier texte clair en texte binaire
void transformFile(char *file_to_transform, char *folder_output) {
	FILE *in = fopen(file_to_transform, "r");
	if (in == NULL) {
		printf("Fichier introuvable\n");
		exit(EXIT_FAILURE);
	}
	char* fileoutname = (char*) malloc ((strlen(folder_output) + strlen(file_to_transform)+5) * sizeof(char));
	strcpy(fileoutname, folder_output);
	strcat(fileoutname, file_to_transform);
	strcat(fileoutname, "_gmp");
	FILE *out = fopen(fileoutname, "wb");
	mpz_t biginteger;
	mpz_init(biginteger);

	while(1) {
		int res = gmp_fscanf(in, "%Zd", biginteger);
		if (res == EOF) {
			break;
		} else if (res != 1) {
			printf("Erreur de lecture\n");
			exit(EXIT_FAILURE);
		} else {
			mpz_out_raw(out, biginteger);	
		}
	}
	fclose(in);
	fclose(out);
}

// Fonction ajoutant une ligne supplémentaire dans un fichier d'impaire élément
void evenFile(char *even_file_name) {
	FILE *in = fopen(even_file_name, "ab");
	if (in == NULL) {
		printf("Fichier introuvable\n");
		exit(EXIT_FAILURE);
	}
	printf("Make File even...");
	mpz_t one;
	mpz_init(one);
	mpz_set_ui(one, 1);
	mpz_out_raw(in, one);
	printf("Done!\n");
	fclose(in);
}

void printFinalProduct() {
	FILE* out2 = fopen("./fact_output/PFinal", "rb");
	mpz_t bigintegergmp;
	mpz_init(bigintegergmp);
	
	mpz_inp_raw(bigintegergmp, out2);
	gmp_printf("Final : %Zd\n", bigintegergmp);

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

	// On compte le nombre de ligne (doit être > 2)
	int listKeys = countLines(moduli);
	if (listKeys <= 2) {
		printf("Votre fichier ne contient pas assez d'éléments\n");
		return 0;
	}
	// Definition du nom du dossier
	char* folder_output = (char*) malloc (15 * sizeof(char));
	strcpy(folder_output, "./fact_output/");

	// On génère le fichier _gmp (binaire)
	transformFile(moduli_filename, folder_output);

	// Nouveau nom fichier _gmp (binaire)
	char* moduli_filename_gmp = (char*) malloc ((strlen(folder_output) + strlen(moduli_filename) + 5) * sizeof(char));
	strcpy(moduli_filename_gmp, folder_output);
	strcat(moduli_filename_gmp, moduli_filename);
	strcat(moduli_filename_gmp, "_gmp");

	// Rajoute une ligne si liste des clefs impaires
	if (listKeys % 2 != 0) {
		printf("Odd Source File...\n");
		evenFile(moduli_filename_gmp);
		listKeys += 1;
	}

	// Nouveau fichier à utiliser : le fichier binaire
	moduli = fopen(moduli_filename_gmp, "rb");
	if (moduli == NULL) {
		printf("Erreur : %s\n", moduli_filename_gmp);
		exit(EXIT_FAILURE);
	}
	// Valeurs récupérant les données deux par deux sur notre fichier en lecture
	mpz_t val1, val2;
	mpz_init(val1);
	mpz_init(val2);
	// Produit de val1 et val2
	mpz_t res_product;
	mpz_init(res_product);

	// Fichier de sortie intermédiaire (au moins un)
	char* sortie_interm_src = (char*) malloc ((strlen(folder_output) + 10) * sizeof(char));
	char* sortie_interm = (char*) malloc ((strlen(sortie_interm_src) + 100) * sizeof(char));
	// Source : Repertoire + nom_fichier_interne
	strcpy(sortie_interm_src, folder_output);
	strcat(sortie_interm_src, "PInterm_");
	// Destination : nom_fichier_interne + level
	strcpy(sortie_interm, sortie_interm_src);
	strcat(sortie_interm, "1");

	// Fichier contenant le produit final
	char* sortie_final= (char*) malloc ((strlen(folder_output) + 10) * sizeof(char));
	strcpy(sortie_final, folder_output);
	strcat(sortie_final, "PFinal");
	
	// Creation premier fichier intermédiaire
	printf("Creating file %s\n", sortie_interm);
	FILE* tmp = fopen(sortie_interm, "wb");
	if (tmp == NULL) {
		fclose(tmp);
		printf("Impossible de crée le fichier %s \n", sortie_interm);
		exit(EXIT_FAILURE);
	}

	// Level i : Nombre de fichiers
	// Level 0 : Fichier final
	int i = 1;
	char* str_level = (char*) malloc (100 * sizeof(char));

	// taille de données lues
	size_t raw_val1;
	size_t raw_val2;

	// Boucle infini
	printf("Starting...\n");
	while(1) {
		printf("Level %d...\n", i);
		
		// On récupère les deux prochaines valeurs du fichier en lecturess
		mpz_init(val1);
		mpz_init(val2);
		raw_val1 = mpz_inp_raw(val1, moduli);
		raw_val2 = mpz_inp_raw(val2, moduli);
		
		// [DEBUG]
		gmp_printf("Sized val 1: %zu -- %Zd \n", raw_val1, val1);
		gmp_printf("Sized val 2: %zu -- %Zd \n", raw_val2, val2);

		// [DEBUG]
		//printf("Test if file is NULL (var1)\n");
		if (raw_val1 == 0 && raw_val2 == 0) {
			printf("EOF\n");
			fclose(moduli);
			fclose(tmp);

			// Calcule le prochain niveau
			listKeys = listKeys/2;

			// [DEBUG]
			//printf("Next level %d\n", listKeys);
			if (listKeys == 1) {
				// Nothing else to do, break out of loop
				break;
			} else {
					// Niveau actuel
					strcpy(str_level, "");
					sprintf(str_level, "%d", i);
					// Si fichier intermédiaire impaire
					if (listKeys % 2 != 0) {
						// [DEBUG]
						//printf("Odd File found...\n");
						strcpy(sortie_interm, "");
						strcpy(sortie_interm, sortie_interm_src);
						evenFile(strcat(sortie_interm, str_level));
						listKeys += 1;
					}

					// [DEBUG]
					//printf("Intermediate becomes main...\n");
					strcpy(sortie_interm, "");
					strcpy(sortie_interm, sortie_interm_src);
					strcat(sortie_interm, str_level);

					moduli = fopen(sortie_interm, "rb");
					if (moduli == NULL) {
							printf("Erreur ouverture fichier intermédiaire %s\n", sortie_interm);
							exit(EXIT_FAILURE);
					}

					// Si dernier fichier intermédiaire (2 lignes restantes)
					if (listKeys == 2) {
						// On ouvre le fichier final en écriture
						printf("Create Final...\n");
						i = 0;
						
						tmp = fopen(sortie_final, "wb");
						if (tmp == NULL) {
							printf("Erreur création fichier final\n");
							exit(EXIT_FAILURE);
						}
					} else {
						// Nouveau fichier intermédiaire avec level + 1
						i++;
						printf("Creating new intermediate file : Level %d...\n", i);
						strcpy(str_level, "");
						sprintf(str_level, "%d", i);
						strcpy(sortie_interm, "");
						strcpy(sortie_interm, sortie_interm_src);
						strcat(sortie_interm, str_level);
						printf("Creating file %s\n", sortie_interm);
						tmp = fopen(sortie_interm, "wb");
						if (tmp == NULL) {
							printf("Erreur création intermédiaire\n");
							exit(EXIT_FAILURE);
						}
					}
			} 
		} else if (raw_val1 != 0 && raw_val2 == 0) {
			printf("Erreur Fichier : Check file for \\n \n");
			exit(EXIT_FAILURE);
		} else {
			mpz_mul(val1, val1, val2);
			mpz_init_set(res_product, val1);
			mpz_out_raw(tmp, res_product);
		}
	}

	printf("THE END...\n");

	return 0;
}

void createOutputFolder() {
	printf("Check folder...");
	struct stat st = {0};
	if (stat("./fact_output", &st) == -1) {
		printf("creating...");
    	mkdir("./fact_output", 0700);
	}
	printf("Done!\n");
}

int buildRemainderTree (int level) {
	return 0;
}

int main(int argc, char** argv) {
	printf("Mainly...\n");
	createOutputFolder();
	buildProductTree(argv[1]);
	printf("Finally...\n");
	printFinalProduct();
	return 0;
}