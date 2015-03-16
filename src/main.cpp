#include "q24.h"

/**
 * Prints the hash to standard output.
 * @param hash [description]
 */
void printHash(uint8_t *hash) {
	for (int i = 0; i < HASH_LENGTH; i++)
		printf("%02x", hash[i]);
	printf("\n");
}

/**
 * Reminds the user how to use the program.
 */
int printUsage(char *bin) {
	cout << "Usage: " << bin << " [options]" << endl;
	cout << "\t-h, --help\t\tPrint usage" << endl;
	cout << "\t-v \t\t\tVerbose output" << endl;
	cout << "\t-i <string>\t\tUses the specified plaintext string as hash input" << endl;
	cout << "\t-f <filename>\t\tUses the specified file as hash input" << endl;
	cout << "\t-o <filename>\t\t(Optional) Writes the output hash to file" << endl;
	cout << "\t-s \t\t\tDon't print output hash to std out" << endl;
	return 0;
}

/**
 * Entry point to the command-line.
 * @param  argc 
 * @param  argv 
 * @return 
 */
int main(int argc, char *argv[]) {

	// Some data to keep track of writing the output hash to a file.
	bool writeOutputToFile = false;
	bool writeOutputToStdout = true;
	char *outputFilename;

	// Some data to hold the input, whether it's a file of bytes or
	// just a string.
	uint8_t *entropy;
	int entropy_length;
	bool entropy_ready = false;

	// Whether or not to print all sorts of entertaining stuff about
	// the quest. The integer determines the level of verbosity.
	int verbose = 0;

	for (int i = 1; i < argc; i++) {

		char *arg = argv[i];

		//---------------------------------------------------------------------//
		if (strcmp(arg, "--help") == 0 || strcmp(arg, "-h") == 0) {

			return printUsage(argv[0]);
		}
		//---------------------------------------------------------------------//
		else if (strcmp(arg, "-i") == 0) {

			if (entropy_ready)
				return printUsage(argv[0]);

			// User has passed in a string as the hash input.
			// Try to parse out the string and turn it into 
			// bytes.
			if (++i >= argc)
				return printUsage(argv[0]);

			entropy_length = strlen(argv[i]);

			// Make sure the string is a sane size.
			if (entropy_length < 0 || entropy_length > 2048) {
				cout << "String input too short or too long!" << endl;
				return 0;
			}

			// Initialize entropy from the string input.
			entropy = new uint8_t[entropy_length];
			strncpy((char *)entropy, argv[i], entropy_length);
			entropy_ready = true;
		}
		//---------------------------------------------------------------------//
		else if (strcmp(arg, "-f") == 0) {

			if (entropy_ready)
				return printUsage(argv[0]);

			// User has specified a file for the hash input.
			// Locate and parse out the file into bytes.
			if (++i >= argc)
				return printUsage(argv[0]);

			struct stat st;
		    if(stat(argv[i], &st) != 0) {
		    	cout << argv[0] << ": " << argv[i] << ": No such file or directory" << endl;
		    	return 0;
		    }

		    entropy_length = (int)st.st_size;

		    if (entropy_length < 1) {
		    	cout << "File size is too small or too big." << endl;
		    	return 0;
		    }

			ifstream infile(argv[1], ios::in | ios::binary);

			entropy = new uint8_t[entropy_length];
			infile.read((char *)entropy, entropy_length);
			infile.close();

			entropy_ready = true;

		}
		//---------------------------------------------------------------------//
		else if (strcmp(arg, "-o") == 0) {

			// User has specified the output to be written to a file
			// in addition to stdout.
			if (++i >= argc)
				return printUsage(argv[0]);

			writeOutputToFile = true; cout << i << endl;
			outputFilename = argv[i];
		}
		//---------------------------------------------------------------------//
		else if (strstr(arg, "-v") != NULL) {

			if (strcmp(arg, "-v") == 0) verbose = 1;
			else if (strcmp(arg, "-vv") == 0) verbose = 2;
			else if (strcmp(arg, "-vvv") == 0) verbose = 3;
			else return printUsage(argv[0]);

		}
		//---------------------------------------------------------------------//
		else if (strcmp(arg, "-s")) {

			// Hide the output.
			writeOutputToStdout = false;
		
		}
		//---------------------------------------------------------------------//
		else {

			// Unrecognized cmd line arg or some string was entered without the
			// preceeding usage flag.
			return printUsage(argv[0]);
		}
	}

	// Make sure that all the necessary flags were specified.
	if (!entropy_ready)
		return printUsage(argv[0]);

	uint8_t *hash = compute(entropy, entropy_length, verbose);

	// Write file to standard output.
	if (writeOutputToStdout) {
		printHash(hash);
	}

	// Write file and leave.
	if (writeOutputToFile) {

		cout << "Writing hash output to " << outputFilename << "..." << endl;

		ofstream outfile(outputFilename, ios::out | ios::binary);
		outfile.write((char *)hash, HASH_LENGTH);
		outfile.close();
	}

	delete entropy;
	delete hash;
}