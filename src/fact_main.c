#include "fact.h"

int input_type = INPUT_HEXA;

int main(int argc, char *argv[]) {
	int c;
	
	int mode = -1;
               
    char *filename = NULL;
	while (1) {
	   int option_index = 0;
	   struct option long_options[] = {
                   {"files",   no_argument, &mode, 0 },
                   {"fullram",  no_argument,    &mode,  1 },
                   {"moduli",  required_argument, 0,  'm' },
                   {"format",  required_argument, 0,  'f' },
                   {0,         0,                 0,  0 }
               };
	   c = getopt_long(argc, argv, "fm:",
	   long_options, &option_index);
	   if (c == -1)
		   break;

	   switch (c) {
	   case 'm':
			filename = optarg;
			break;
	   case 'f':
			if (strcmp (optarg, "decimal") == 0) {
				printf ("decimal\n");
				input_type = INPUT_DECIMAL;
				printf ("input format set\n");
			} else {
				fprintf (stderr, "Unknown input format: %s\n", optarg);
				exit (EXIT_FAILURE);
			}
			break;
	   default:
			break;
	   }
	}
	
	if (mode == -1) {
		fprintf (stderr, "Usage: %s -m|--moduli inputfile --files|--fullram  [--format hexa|decimal]\n", argv[0]);
		exit (EXIT_FAILURE);
	}

	if (mode == MODE_FILES) {
		struct stat st = {0};
		if(stat("output_gmp", &st) != 0) {
			mkdir("output_gmp", 0700);
		}
		buildProductTree(filename);
		printFinalProduct();
		buildRemainderTree ();
	} else if (mode == MODE_FULLRAM) {
		computeSuperSpeed (filename);
	} else {
		fprintf (stderr, "Usage: %s -m|--moduli inputfile --files|--fullram\n", argv[0]);
		exit (EXIT_FAILURE);
	}
	exit (EXIT_SUCCESS);
}
