

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include "ClassParser.hpp"
#include "ByteCodeGen.hpp"
#include "ByteCodeGen_X86_64.hpp"
#include "SnapJVMRuntime.hpp"

void printUsage(char * argv0)
{
	fprintf(stderr, "Usage\n");
	fprintf(stderr, "  %s [-c] <filename>.class\n", argv0);
    fprintf(stderr, "Example:\n");
    fprintf(stderr, "  %s -c -v Hello\n", argv0);
    fprintf(stderr, "Optiones:\n");
    fprintf(stderr, "    -c - Print class information\n");
    fprintf(stderr, "    -n - Don't execute\n");
    fprintf(stderr, "    -v - Verbose mode\n");
    fprintf(stderr, "Optiones:\n");
}

int main(int argc, char **argv) {
	ClassParser classParser;

    int opt;
    bool printClassInfo = false;
    bool executeMain = true;
    SnapJVMRuntime::setVerboseMode(false);

    while ((opt = getopt(argc, argv, "cnv")) != -1) {
        switch (opt) {
        case 'c':
        	printClassInfo = true;
            break;
        case 'n':
        	executeMain = false;
			break;
        case 'v':
        	SnapJVMRuntime::setVerboseMode(true);
			break;
        default: /* '?' */
            fprintf(stderr, "Usage: %s [-c] classFile\n", argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    if (optind >= argc) {
    	printUsage(argv[0]);
        exit(EXIT_FAILURE);
    }

    char * classFile = argv[optind];

	ByteCode::initialize();

	ClassClass * classClass = classParser.parse(classFile);

	if (printClassInfo) {
		classClass->print();
	}

	ByteCodeGen * byteCodeGen = new ByteCodeGen_X86_64(classClass);
	byteCodeGen->codeGen();

	if (executeMain) {
		int ret = classClass->runMain(optind+1, &argv[optind+1]);
		return ret;
	}

	return 0;
}
