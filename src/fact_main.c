#include "fact.h"

int main(int argc, char *argv[]) {
	//~ printf("Checking folder...");
	//~ struct stat st = {0};
	//~ if(stat("output_gmp", &st) != 0) {
		//~ printf("...creating...");
		//~ mkdir("output_gmp", 0700);
	//~ }
	//~ printf("Ready!\n");
	//~ buildProductTree(strdup(argv[1]));
	//~ printf("\nFinally...\n");
	//~ printFinalProduct();
	//~ buildRemainderTree ();
	computeSuperSpeed (argv[1]);
	return 0;
}
