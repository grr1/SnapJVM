/*
 * AssemblerX86_64.cpp
 *
 *  Created on: Nov 8, 2019
 *      Author: grr
 */

#include "AssemblerX86_64.hpp"
#include "keystone/keystone/keystone.h"
#include "SnapJVMRuntime.hpp"

#include <stdio.h>
#include <sys/mman.h>
#include <string.h>


Assembler_X86_64::Assembler_X86_64() {
	ks_err err = ks_open(KS_ARCH_X86, KS_MODE_64, &ks);
    if (err != KS_ERR_OK) {
        printf("ERROR: failed on ks_open(), quit\n");
        return;
    }

    ks_option(ks, KS_OPT_SYNTAX, KS_OPT_SYNTAX_ATT);

    // Initialize mmap allocator
    _mapSizeIncrement = 1024*1024;
    _mapSizeLeftBeforeIncrement = 4*1024;
    _mapSize = 0;
    _mapSizeLeft = 0;
    _codeBufferStart = NULL;
    _codeBufferCurrent = NULL;
    _codeBufferCurrent = NULL;
}

Assembler_X86_64::~Assembler_X86_64() {
}

unsigned char *
Assembler_X86_64::assemble(const char * codeStr)
{
	if (_mapSizeLeft < _mapSizeLeftBeforeIncrement) {
		// Allocate executable memory
		_mapSize= _mapSizeIncrement;
		_codeBufferStart =  (unsigned char * ) mmap(NULL, _mapSize, PROT_READ|PROT_WRITE|PROT_EXEC,
				MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
		if ( (void *) _codeBufferStart == MAP_FAILED) {
			printf("mmap failed\n");
			exit(1);
		}

		_mapSizeLeft = _mapSize;
		_codeBufferCurrent = _codeBufferStart;
	}

	unsigned char *encode = _codeBufferCurrent;
	size_t size = _mapSizeLeft;
	size_t count = 0;
	if (ks_asm(ks, codeStr, (unsigned long int) encode, &encode, &size, &count) != KS_ERR_OK) {
		printf("ERROR: ks_asm() failed & count = %lu, error = %u\n",
			 count, ks_errno(ks));
	}
	else
	{
		size_t i;

		if (SnapJVMRuntime::isVerboseMode()) {
			printf("%s = ", codeStr);
			for (i = 0; i < size; i++) {
			  printf("%02x ", encode[i]);
			}
			printf("\n");
			printf("Compiled: %lu bytes, statements: %lu\n", size, count);
		}
	}
	memcpy(_codeBufferCurrent, encode, size);

    unsigned char * retAddr = _codeBufferCurrent;

	_codeBufferCurrent += size;
	_mapSizeLeft -= size;
	if ( _mapSizeLeft < 0) {
		printf("Assembler_X86_64::assemble: Max increment not enough\n");
	}
	return retAddr;
}

