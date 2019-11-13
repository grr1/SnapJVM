PROJECT_ROOT = $(dir $(abspath $(lastword $(MAKEFILE_LIST))))
LIBS= -L$(PROJECT_ROOT)/keystone -lkeystone

SRCS=  snap-jvm.cpp ClassParser.cpp ByteCode.cpp ClassClass.cpp ByteCodeGen.cpp ByteCodeGen_X86_64.cpp \
	   Method.cpp AssemblerX86_64.cpp SnapJVMRuntime.cpp
	  
HDRS= AssemblerX86_64.hpp ByteCodeGen_X86_64.hpp ClassClass.hpp CodeGen.hpp Object.hpp \
       ByteCodeGen.hpp ByteCode.hpp ClassParser.hpp Method.hpp SnapJVMRuntime.hpp

OBJS=  snap-jvm.o ClassParser.o ByteCode.o ClassClass.o ByteCodeGen.o ByteCodeGen_X86_64.o \
	   Method.o AssemblerX86_64.o SnapJVMRuntime.o

CFLAGS += -g

all:	snap-jvm

snap-jvm:	$(OBJS) $(HDRS)
	$(CXX) -o $@ $(OBJS) $(LIBS)

%.o:	$(PROJECT_ROOT)%.cpp $(HDRS)
	$(CXX) -c $(CFLAGS) $(CXXFLAGS) $(CPPFLAGS) -o $@ $<

%.o:	$(PROJECT_ROOT)%.c
	$(CC) -c $(CFLAGS) $(CPPFLAGS) -o $@ $<

clean:
	rm -fr snap-jvm $(OBJS)
