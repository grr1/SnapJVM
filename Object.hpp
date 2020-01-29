/*
 * Object.h
 *
 *  Created on: Nov 1, 2019
 *      Author: grr
 */

#ifndef OBJECT_H_
#define OBJECT_H_

class Object {
public:
    ClassClass * _class;
    void * _localVariables;
    Object(ClassClass * myClass);
	getField(string);
    Object();
	virtual ~Object();
};

#endif /* OBJECT_H_ */
