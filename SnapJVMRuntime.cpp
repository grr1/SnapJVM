/*
 * SnapJVMRuntime.cpp
 *
 *  Created on: Nov 7, 2019
 *      Author: grr
 */
#include <stdio.h>

#include "SnapJVMRuntime.hpp"

SnapJVMRuntime * SnapJVMRuntime::_theJVMRuntime;
bool SnapJVMRuntime::_verboseMode;
bool SnapJVMRuntime::_testMode;

SnapJVMRuntime::SnapJVMRuntime() {
	// TODO Auto-generated constructor stub
	_runtimeDispatchTable[JVMRuntime_invokevirtual] = runtime_invokevirtual;
}

SnapJVMRuntime::~SnapJVMRuntime() {
	// TODO Auto-generated destructor stub
}

void *
SnapJVMRuntime::runtime_invokevirtual(SnapJVMRuntime * runTime, Object * o, void *a, void *b, void *c, void *d)
{
	printf("%s", (char *) a);
}

int calculate_size(u8 type){
  switch(type){
  case T_BOOLEAN:
  case T_BYTE:
    return 1;
    break;
  case T_SHORT:
  case T_CHAR:
    return 2;
    break;
  case T_INT:
  case T_FLOAT:
    return 4;
  case T_DOUBLE:
  case T_LONG:
    return 8;
  }
  return 8;
}

void * SnapJVMRuntime::runtime_newarray(u8 type, u8 length){
  if(((int)length) <0){
    //should throw a NegativeArraySizeException
    //print out the error message for now
    fprintf(stderr, "NegativeArraySizeException: %d\n", (int)length);
  }
  int type_size = calculate_size(type);
  //calculate size needed
  int size = 8 + length * type_size; //use 8 bytes to store length

  //allocation. Using malloc for now, will switch to our own implementation
  void* arrayPointer = malloc(size);
  //initialize values
  char* e = (char*)arrayPointer;
  long* lengthPtr = (long*)e;
  *lengthPtr = length;
  e += sizeof(long);
  //initialize float/double with positive zeros, boolean with false, the others with zeros
  for(int i = 0; i < length*type_size; i++){
    *e++ = 0;
  }
  return arrayPointer;
}

u8 SnapJVMRuntime::runtime_array_load(u8 type, u8 index, u8 reference){
  //TODO: check whether reference is null here; if so, throw ArrayIndexOutOfBoundsException
  char* arrayPointer = (char*)reference;
  long length = *(long*)arrayPointer;
  if(index < 0 || index >=length){
    //should throw a ArrayIndexOutOfBoundsException
    //print out the error message for now
    fprintf(stderr, "ArrayIndexOutOfBoundsException: %d\n", (int)length);
  }
  int type_size = calculate_size(type);
  arrayPointer += sizeof(long) + type_size*index;
  switch (type_size){
  case 1:{
    u1 k = *(u1*)arrayPointer;
    return (u8)k;
  }
  case 2:{
    u2 k = *(u2*)arrayPointer;
    return (u8)k;
  }
  case 4:{
    u4 k = *(u4*)arrayPointer;
    return (u8)k;
  }
  case 8:{
    u8 k = *(u8*)arrayPointer;
    return k;
  }
  }
  return 0;
}

void SnapJVMRuntime::runtime_array_store(u8 type, u8 value, u8 index, u8 reference){
  //TODO: check whether reference is null here; if so, throw ArrayIndexOutOfBoundsException
  char* arrayPointer = (char*)reference;
  long length = *(long*)arrayPointer;
  if(index < 0 || index >=length){
    //should throw a ArrayIndexOutOfBoundsException
    //print out the error message for now
    fprintf(stderr, "ArrayIndexOutOfBoundsException: %d\n", (int)length);
  }
  int type_size = calculate_size(type);
  arrayPointer += sizeof(long) + type_size*index;
  switch (type_size){
  case 1:{
    u1* k = (u1*)arrayPointer;
    *k = (u1)value;
  }
    break;
    
  case 2:{
    u2* k = (u2*)arrayPointer;
    *k = (u2)value;
  }
    break;
  case 4:{
    u4* k = (u4*)arrayPointer;
    *k = (u4)value;
  }
    break;
  case 8:{
    u8* k = (u8*)arrayPointer;
    *k = (u8)value;
  }
    break;
  }
}

