/*
 * ClassParser.hpp
 *
 *  Created on: Oct 6, 2019
 *      Author: grr
 */

#ifndef CLASSPARSER_HPP_
#define CLASSPARSER_HPP_

#include <sys/types.h>
#include <stdio.h>

#include "ByteCode.hpp"

class ClassClass;

class ClassParser {
	friend class ByteCodeGen_X86_64;
private:

	ClassClass * _classClass; // Class currently parsed

	char * _classFileName;
	size_t _classFileLength;
	unsigned char * _classFileBuffer; // We read entire file into memory. Only set during parsing.
	unsigned char *_u;  // Current position

	static bool _isLittleEndianInitialized;
	static bool _isLittleEndian;

	u1 readU1();
	u2 readU2();
	u4 readU4();
	u8 readU8();

	static bool isLittleEndian();

public:
	ClassParser();
	ClassClass * parse(const char * classFileName);
	bool parseConstantPool();
	bool parseInterfaces();
	bool parseFields();
	bool parseMethods();
	static u1 readU1(unsigned char * & uptr);
	static u2 readU2(unsigned char * & uptr);
	static u4 readU4(unsigned char * & uptr);
	static u8 readU8(unsigned char * & uptr);
};

u1
inline ClassParser::readU1(unsigned char * & _u) {
	u1 val = *_u;
	_u++;
	return val;
}

u2
inline ClassParser::readU2(unsigned char * & _u) {
	u2 val;
	u1 * u = (u1 *) &val;
	if (isLittleEndian()) {
		u[1]=*_u;_u++;
		u[0]=*_u;_u++;
	}
	else {
		u[0]=*_u;_u++;
		u[1]=*_u;_u++;
	}
	return val;
}

u4
inline ClassParser::readU4(unsigned char * & _u) {
	u4 val;
	u1 * u = (u1 *) &val;
	if (isLittleEndian()) {
		u[3]=*_u;_u++;
		u[2]=*_u;_u++;
		u[1]=*_u;_u++;
		u[0]=*_u;_u++;
	}
	else {
		u[0]=*_u;_u++;
		u[1]=*_u;_u++;
		u[2]=*_u;_u++;
		u[3]=*_u;_u++;
	}
	return val;
}

u8
inline ClassParser::readU8(unsigned char * & _u) {
	u8 val;
	u1 * u = (u1 *) &val;
	if (isLittleEndian()) {
		u[7]=*_u;_u++;
		u[6]=*_u;_u++;
		u[5]=*_u;_u++;
		u[4]=*_u;_u++;
		u[3]=*_u;_u++;
		u[2]=*_u;_u++;
		u[1]=*_u;_u++;
		u[0]=*_u;_u++;
	}
	else {
		u[0]=*_u;_u++;
		u[1]=*_u;_u++;
		u[2]=*_u;_u++;
		u[3]=*_u;_u++;
		u[4]=*_u;_u++;
		u[5]=*_u;_u++;
		u[6]=*_u;_u++;
		u[7]=*_u;_u++;
	}
	return val;
}

u1
inline ClassParser::readU1() {
	return readU1(_u);
}

u2
inline ClassParser::readU2() {
	return readU2(_u);
}

u4
inline ClassParser::readU4() {
	return readU4(_u);
}

u8
inline ClassParser::readU8() {
	return readU8(_u);
}

bool
inline ClassParser::isLittleEndian() {
	if (_isLittleEndianInitialized) {
		return _isLittleEndian;
	}
	int e = 5;
	_isLittleEndian = ((*(char*)&e) == 5);
	return _isLittleEndian;
}

#endif /* CLASSPARSER_HPP_ */


