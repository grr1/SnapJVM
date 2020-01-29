/*
 * Object.h
 *
 *  Created on: Nov 1, 2019
 *      Author: grr
 */

#ifndef OBJECT_H_
#define OBJECT_H_
#include <map>
#include <string>
#include "ClassClass.hpp"

class Object {
public:
    ClassClass * _class;
    void * _localVariables;
    Object(ClassClass * myClass);
	u8 getField(std::string);
	void addField(std::string, u8);
    Object();
	virtual ~Object();
};

#endif /* OBJECT_H_ */
