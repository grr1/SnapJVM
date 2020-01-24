/*
 * ObjectAllocator.cpp
 *
 *  Created on: Jan 24, 2020
 *      Author: grr
 */

#include "ObjectAllocator.h"

ObjectAllocator::ObjectAllocator() {
	// TODO Auto-generated constructor stub

}

ObjectAllocator::~ObjectAllocator() {
	// TODO Auto-generated destructor stub
}

Object *
ObjectAllocator::allocate(u8 bytes)
{
	return (Object *) malloc(bytes);
}

