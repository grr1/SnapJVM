/*
 * ByteCodeGen_X86_64.h
 *
 *  Created on: Nov 5, 2019
 *      Author: grr
 */

#ifndef BYTECODEGEN_X86_64_HPP_
#define BYTECODEGEN_X86_64_HPP_

#include "ByteCodeGen.hpp"
#include "ClassClass.hpp"
#include "AssemblerX86_64.hpp"

#include <string>

class ByteCodeGen_X86_64 : public ByteCodeGen {
	ClassClass * _classClass;
	std::stringstream * _codeStrStream;
	Assembler_X86_64 * _assembler_X86_64;

	int _top;
	int _maxStackRegs;
	const char * _stackRegs[16];
	int _maxArgumentRegs;
	const char * _argumentRegs[16];
	u2 _max_stack;
	u2 _max_locals;
	u2 _max_args;
	u4 _code_length;

	Method * _method;

	int _stackFrameArgs;
	int _stackFrameCalleeSaved;
	int _stackFrameLocalVars;
	int _stackFRameJavaStack;
	int _stackFrameEnd;

public:
	ByteCodeGen_X86_64(ClassClass * classClass);
	virtual ~ByteCodeGen_X86_64();
	void codeGen();
	void codeGenOne(ByteCode::Code code, u1 * codeArray, int k);
	void notImplemented(ByteCode::Code code);
	void restoreRegsBeforeReturn();

	void pushVirtualStack();
	void popVirtualStack();
	const char * getReg();
};

#endif /* BYTECODEGEN_X86_64_HPP_ */
