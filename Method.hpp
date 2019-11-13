/*
 * Method.h
 *
 *  Created on: Nov 1, 2019
 *      Author: grr
 */

#ifndef METHOD_H_
#define METHOD_H_

#include <string>
#include "ClassClass.hpp"
#include "Object.hpp"

class Method {
public:
		char * _methodName;
		ClassClass * _classClass;
		char *  _codeStr;
		unsigned char * _code;
		unsigned long _maxStack;
		unsigned long _maxLocals;
		unsigned long _maxArgs;
public:
	Method();
	void invoke( Object * o);
	virtual ~Method();
};

#endif /* METHOD_H_ */
