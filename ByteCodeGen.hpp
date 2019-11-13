/*
 * ByteCodeGen.h
 *
 *  Created on: Nov 1, 2019
 *      Author: grr
 */

#ifndef BYTECODEGEN_HPP_
#define BYTECODEGEN_HPP_

#include "ClassParser.hpp"
#include "Method.hpp"

class ByteCodeGen {
protected:
	ClassParser * _classParser;
public:
	ByteCodeGen();
	virtual void codeGen();
	virtual ~ByteCodeGen();
};

#endif /* BYTECODEGEN_HPP_ */
