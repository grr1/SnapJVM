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
