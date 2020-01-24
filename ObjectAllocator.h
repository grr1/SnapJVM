/*
 * ObjectAllocator.h
 *
 *  Created on: Jan 24, 2020
 *      Author: grr
 */

#ifndef OBJECTALLOCATOR_H_
#define OBJECTALLOCATOR_H_

#include "ByteCode.hpp"
#include "Object.h"

class ObjectAllocator {
public:
	ObjectAllocator();
	virtual ~ObjectAllocator();

	Object * allocate(u8 bytes);
};

#endif /* OBJECTALLOCATOR_H_ */
