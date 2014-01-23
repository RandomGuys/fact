#include <stdlib.h>
#include <stdio.h>
#include <gmp.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>

int main(int argc, char** argv) {
	if (argc != 2) {
		printf("Il faut un argument à la fonction - un chemin vers un fichier de la forme\n");
		printf("Syntaxe\n");
		printf("entier_decimal_1\n...\nentier_decimal_N\n");
		exit(EXIT_FAILURE);
	}
	FILE* in = fopen(argv[1], "r");
	if (in == NULL) {
		printf("Fichier introuvable\n");
		exit(EXIT_FAILURE);
	}
	FILE* out = fopen(strcat(argv[1],"_gmp"), "wb");
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
			gmp_printf("%Zd\n", biginteger);
			mpz_out_raw(out, biginteger);	
		}
	}
	fclose(in);
	fclose(out);
	

	/* Contrôle 
	FILE* out2 = fopen(argv[1], "rb");
	mpz_t bigintegergmp;
	mpz_init(bigintegergmp);
	while(mpz_cmp_ui(bigintegergmp, EOF) != 0) {
		mpz_inp_raw(bigintegergmp, out2);
		gmp_printf("%Zd\n", bigintegergmp);
	}

	fclose(out2); */
}
