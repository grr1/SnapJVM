/*
 * AssemblerX86_64.hpp
 *
 *  Created on: Nov 8, 2019
 *      Author: grr
 */

#ifndef ASSEMBLERX86_64_HPP_
#define ASSEMBLERX86_64_HPP_

#include "keystone/keystone/keystone.h"

class Assembler_X86_64 {
    ks_engine *ks;

    // mmap allocator
    unsigned long _mapSizeIncrement;
    unsigned long _mapSizeLeftBeforeIncrement;

    unsigned long _mapSize;
    unsigned long _mapSizeLeft;
    unsigned char * _codeBufferStart;
    unsigned char * _codeBufferCurrent;
public:
	Assembler_X86_64();
	unsigned char * assemble(const char * codeStr);
	virtual ~Assembler_X86_64();
};

#endif /* ASSEMBLERX86_64_HPP_ */
