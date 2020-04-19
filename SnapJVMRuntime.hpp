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

class SnapJVMRuntime;

typedef void * (* InvokeJVMRuntimeFuncPtr)(SnapJVMRuntime * runTime, Object * o, void *a, void *b, void *c, void *d);

enum SnapJVMRuntimeCallTimes{
	JVMRuntime_invokevirtual, // bytecode for _invokevirtual
	JVMRuntime_max
};

class SnapJVMRuntime {
	InvokeJVMRuntimeFuncPtr _runtimeDispatchTable[JVMRuntime_max];

	static bool _verboseMode;
	static bool _testMode;

	///////////////////////////////////////////

public:
	static SnapJVMRuntime * _theJVMRuntime;

	static SnapJVMRuntime * TheJVMRuntime();
	SnapJVMRuntime();
	virtual ~SnapJVMRuntime();

	void invokeMethod( Object * o, char * methodName );
	static void setVerboseMode(bool verboseMode);
	static bool isVerboseMode();
	static void setTestMode(bool testMode);
	static bool isTestMode();

	// Calls runtime
	static void * runtime_invokevirtual(SnapJVMRuntime * runTime, Object * o, void *a, void *b, void *c, void *d);
  static void * runtime_newarray(u8 type, u8 length);
  static u8 runtime_array_load(u8 type, u8 index, u8 reference);
  static void runtime_array_store(u8 type, u8 value, u8 index, u8 reference);
};

inline SnapJVMRuntime *
SnapJVMRuntime::TheJVMRuntime()
{
	return _theJVMRuntime;
}

inline void
SnapJVMRuntime::setVerboseMode(bool verboseMode) {
	_verboseMode = verboseMode;
	if(verboseMode){
	  setbuf(stdout, NULL);
	}
}

inline bool
SnapJVMRuntime::isVerboseMode()
{
	return _verboseMode;
}

inline void
SnapJVMRuntime::setTestMode(bool testMode) {
	_testMode = testMode;
}

inline bool
SnapJVMRuntime::isTestMode() {
	return _testMode;
}

#endif /* SNAPJVMRUNTIME_HPP_ */

