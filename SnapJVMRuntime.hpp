/*
 * SnapJVMRuntime.hpp
 *
 *  Created on: Nov 7, 2019
 *      Author: grr
 */

#ifndef SNAPJVMRUNTIME_HPP_
#define SNAPJVMRUNTIME_HPP_

#include "Object.hpp"

enum JVMRuntimeCallNumber {
	VirtualCall=1
};

typedef void * InvokeJVMRuntimeFuncPtr(long callNumer, void * a, void * b, void * c, void * d, void * e, void * f);

class SnapJVMRuntime {
	// Make sure it is the first member variable since it is called from JIT code
	InvokeJVMRuntimeFuncPtr _invokeJVMRuntime;

	///////////////////////////////////////////

	static SnapJVMRuntime * _theJVMRuntime;

	static bool _verboseMode;

public:
	static SnapJVMRuntime * TheJVMRuntime();
	SnapJVMRuntime();
	virtual ~SnapJVMRuntime();

	void invokeMethod( Object * o, char * methodName );
	static void setVerboseMode(bool verboseMode);
	static bool isVerboseMode();
};

inline SnapJVMRuntime *
SnapJVMRuntime::TheJVMRuntime()
{
	return _theJVMRuntime;
}

inline void
SnapJVMRuntime::setVerboseMode(bool verboseMode) {
	_verboseMode = verboseMode;
}

inline bool
SnapJVMRuntime::isVerboseMode()
{
	return _verboseMode;
}

#endif /* SNAPJVMRUNTIME_HPP_ */

