/*
 * ByteCodeGen_X86_64.cpp
 *
 *  Created on: Nov 5, 2019
 *      Author: grr
 */

#include "ByteCodeGen_X86_64.hpp"
#include "Method.hpp"
#include "AssemblerX86_64.hpp"
#include "SnapJVMRuntime.hpp"

#include <string>
#include <sstream>
#include <string.h>

ByteCodeGen_X86_64::ByteCodeGen_X86_64(ClassClass * classClass) {
	_classClass = classClass;
	_assembler_X86_64 = new Assembler_X86_64();
	_top = 0;

	_stackRegs[0]="rbx";
	_stackRegs[1]="r12";
	_stackRegs[2]="r13";
	_stackRegs[3]="r14";
	_stackRegs[4]="r15";
	_maxStackRegs = 5;

	// rdi - 1st argument - SnapJVMRuntime::TheJVMRuntime()
	// rsi - 2nd argument - Object * o
	// rdx - 3rd argument - 1st method argument
	// rcx - 4th argument - 2nd method argument
	// r8  - 5th argument - 3rd method argument
	// r9  - 6th argument - 4th method argument

	_argumentRegs[0]="rdi";
	_argumentRegs[1]="rsi";
	_argumentRegs[2]="rdx";
	_argumentRegs[3]="rcx";
	_argumentRegs[4]="r8";
	_argumentRegs[5]="r9";
	_maxArgumentRegs = 6;
}

ByteCodeGen_X86_64::~ByteCodeGen_X86_64() {

}

void
ByteCodeGen_X86_64::pushVirtualStack()
{
	_top++;
	if (_top >= _maxStackRegs) {
		// Save register
		*this->_codeStrStream << "       movq   %" << getReg() << "," << "-"
				<< 8*(_stackFRameJavaStack + _top) << "       # save top="<< _top <<"\n";
	}
}

void
ByteCodeGen_X86_64::popVirtualStack()
{
	_top--;
	if (_top >= _maxStackRegs) {
		// Save register
		*this->_codeStrStream << "       movq   "
				<< "-" << 8*(_stackFRameJavaStack + _top) << ",%"  << getReg()
				<< "       # save top="<< _top <<"\n";
	}

	if (_top < 0) {
		printf("********************** ERROR *******************\nTop=%d\n", _top);
		printf("Code: %s\n", this->_codeStrStream->str().c_str());
	}
}

const char *
ByteCodeGen_X86_64::getReg()
{
	return _stackRegs[_top % _maxStackRegs];
}

void
ByteCodeGen_X86_64::codeGen()
{
	if (SnapJVMRuntime::isVerboseMode()) {
		printf("--------------------------------------------------\n");
		printf("Code Generation:\n");
		printf("--------------------------------------------------\n");
	}

	_classClass->_methodsArray = new Method[_classClass->_methods_count];

	_codeStrStream = new std::stringstream();

	// Print methods
	for (int i = 0; i < _classClass->_methods_count; i++) {
		// Initialize string
		_method = &_classClass->_methodsArray[i];

		MethodInfo * methodInfo = _classClass->_methodInfoArray[i];

		if (SnapJVMRuntime::isVerboseMode()) {
			// Print type
			printf("Type name:"); _classClass->printExtendedType(methodInfo->descriptor_index);
			printf("Method Name:"); _classClass->printStringDecorated(methodInfo->name_index);
		}

		ConstantPoolInfo *info = _classClass->_constantPoolInfoArray[methodInfo->name_index];

		CONSTANT_Utf8_info * uinfo = dynamic_cast<CONSTANT_Utf8_info *>(info);
		if (uinfo == NULL) {
			printf("(no UTF index #%d)", methodInfo->name_index);
			return;
		}

		_method->_methodName = (char *) uinfo->bytesArray;

		//printf(";\n");
		//printf("    descriptor: ");
		//printStringDecorated(methodInfo->descriptor_index);
		//printf("\n");
		//printf("    flags: (0x%04X)\n",methodInfo->access_flags);
		//printf("    attributes_count: %d\n",methodInfo->attributes_count);
		for (int j = 0; j < methodInfo->attributes_count; j++) {
			AttributeInfo * attrInfo = methodInfo->attributesArray[j];
			if (SnapJVMRuntime::isVerboseMode()) {
				printf("    %d:",j); _classClass->printStringDecorated(attrInfo->attribute_name_index);
				printf("\n");
			}
			//printf("    attributes_count=%d\n", attrInfo->attribute_length);
			//printf("\n");
			ConstantPoolInfo *info = _classClass->_constantPoolInfoArray[attrInfo->attribute_name_index];
			CONSTANT_Utf8_info * uinfo = dynamic_cast<CONSTANT_Utf8_info *>(info);
			const char * sname = "";
			if (uinfo != NULL) {
				sname = (const char *)uinfo->bytesArray;
			}
			if (!strcmp(sname,"Code")) {
				//CodeAttribute * codeAttribute = ()
				unsigned char *uptr = attrInfo->infoArray;
				_max_stack = _classParser->readU2(uptr);
				_max_locals = _classParser->readU2(uptr);
				_code_length = _classParser->readU4(uptr);

				_method->_maxStack = _max_stack;
				_method->_maxLocals = _max_locals;

				u2 max_args = 6;
				_method->_maxArgs = max_args;

				u1 * codeArray = uptr;
				int k = 0;

				_codeStrStream->str("");

				//
				// Every method has the following arguments:
				// rdi - 1st argument - SnapJVMRuntime::TheJVMRuntime()
				// rsi - 2nd argument - Object * o
				// rdx - 3rd argument - 1st method argument
				// rcx - 4th argument - 2nd method argument
				// r8  - 5th argument - 3rd method argument
				// r9  - 6th argument - 4th method argument
                //
				// Stack format
				// --------------
				// arguments saved rdi,rsi, rdx, rcx, r8, r9
				// --------------
				// Callee saved regs "rbx", "r12", "r13", "r14", "r15"
				// --------------
				// Java local vars
				// --------------
				// Java stack
				// --------------
				///

				_stackFrameArgs = 0;
				_stackFrameCalleeSaved = _stackFrameArgs + _maxArgumentRegs;
				_stackFrameLocalVars = _stackFrameCalleeSaved + _maxStackRegs;
				_stackFRameJavaStack = _stackFrameLocalVars + _max_locals;
				_stackFrameEnd = _stackFRameJavaStack + _max_stack;

				*_codeStrStream << "\n";
				*_codeStrStream << _method->_methodName << ":\n";
				*_codeStrStream << "       #max_stack=" << _max_stack
								<< "       max_locals=" << _max_locals << "\n\n";
				*_codeStrStream << "       # Save frame pointer\n";
				*_codeStrStream << "       pushq   %rbp\n";
				*_codeStrStream << "       movq    %rsp, %rbp\n\n";
				*_codeStrStream << "       # Allocate space for locals and stack\n";
				*_codeStrStream << "       subq    $" << std::dec << 8 * (_stackFrameEnd)  << ",%rsp\n";

				// Save argument registers
				*_codeStrStream << "       # Save arguments\n";
				for (int i = 0; i < _maxArgumentRegs; i++) {
					*_codeStrStream << "       mov   "<< _argumentRegs[i] << ",-" << 8*(i+_stackFrameArgs) << "(%rbp)\n";
				}
				*_codeStrStream << "\n";

				// Save callee saved registers
				*_codeStrStream << "\n";
				*_codeStrStream << "       #Save callee saved registers\n";
				for (int i = 0; i < _maxStackRegs; i++) {
					*_codeStrStream << "       mov   "<< _stackRegs[i] << ",-" << 8*(i+_stackFrameCalleeSaved) << "(%rbp)\n";
				}
				*_codeStrStream << "\n";
				while (k<_code_length) {
					*_codeStrStream << "offset_" << std::dec << k << ":\n";
					ByteCode::Code c =  (ByteCode::Code) codeArray[k];

					if (SnapJVMRuntime::isVerboseMode()) {
						printf("         %d: %s ", k, ByteCode::_name[c]);
						u2 flags = ByteCode::_flags[c];
						if (flags & ByteCode::Flags::_fmt_has_j &&
							flags & ByteCode::Flags::_fmt_has_u2	) {
							u1 * p = uptr+1;
							u2 arg = _classParser->readU2(p);
							printf(" #%d",arg);
							ConstantPoolInfoPtr info = _classClass->_constantPoolInfoArray[arg];
							if (info != NULL) { // skip entries with no info
								printf("\t// ");info->printShort(_classClass);
							}
						}
						else if (flags & ByteCode::Flags::_fmt_has_k &&
							flags & ByteCode::Flags::_fmt_has_u2	) {
							u1 * p = uptr+1;
							u2 arg = _classParser->readU2(p);
							printf(" #%d",arg);
							ConstantPoolInfoPtr info = _classClass->_constantPoolInfoArray[arg];
							if (info != NULL) { // skip entries with no info
								printf("\t// ");info->printShort(_classClass);
							}
						}
						else if (flags & ByteCode::Flags::_fmt_has_k &&
								!(flags & ByteCode::Flags::_fmt_has_u2)) {
							u1 * p = uptr+1;
							u1 arg = _classParser->readU1(p);
							printf(" #%d",arg);
							ConstantPoolInfoPtr info = _classClass->_constantPoolInfoArray[arg];
							if (info != NULL) { // skip entries with no info
								printf("\t// ");info->printShort(_classClass);
							}
						}
						printf("\n");
					}

					//printFlags(flags);
					//printf("         %d: %d\n",k, attrInfo->infoArray[k]);
					//k++;

					//*_codeStrStream << "_maxArgumentRegs=" << _maxArgumentRegs << "\n";

					// Generate code for every ByteCode
					codeGenOne(c, codeArray, k);

					k+=ByteCode::_lengths[c];
					uptr += ByteCode::_lengths[c];
				}

				// Done by return
				//*_codeStrStream <<
				//		"       leave\n"
				//		"       ret\n";

				*_codeStrStream << "\n";

				if (SnapJVMRuntime::isVerboseMode()) {

					printf("\n===========================================\n");

					printf("%s\n", this->_codeStrStream->str().c_str());

					printf("\n===========================================\n");
				}

				_method->_codeStr = strdup(this->_codeStrStream->str().c_str());
				_method->_code = _assembler_X86_64->assemble(_method->_codeStr);
				_method->_callMethod = (InvokeJVMRuntimeFuncPtr) _method->_code;
			}
		}
		if (SnapJVMRuntime::isVerboseMode()) {
			printf("\n");
		}
	}
}

void
ByteCodeGen_X86_64::restoreRegsBeforeReturn()
{
	//*_codeStrStream << "_maxArgumentRegs=" << _maxArgumentRegs << "\n";

	// Restore callee saved registers
	*_codeStrStream << "\n";
	*_codeStrStream << "       #Restore callee saved registers\n";
	for (int i = 0; i < _maxStackRegs; i++) {
		*_codeStrStream << "       mov   -"<< std::dec << 8*(i+_stackFrameCalleeSaved)  << "(%rbp), %" << _stackRegs[i] << "\n";
	}
	*_codeStrStream << "\n";
}

void
simplePrintf(const char * s) {
	printf("%s\n",s);
}

void ByteCodeGen_X86_64::codeGenOne(ByteCode::Code code, u1 * codeArray, int k) {
	switch(code) {
	case ByteCode::_nop:{
          notImplemented(code);
        }
        break;

    case ByteCode::_aconst_null:{
          notImplemented(code);
        }
        break;

    case ByteCode::_iconst_m1:{
          notImplemented(code);
        }
        break;

    case ByteCode::_iconst_0:
    case ByteCode::_iconst_1:
    case ByteCode::_iconst_2:
    case ByteCode::_iconst_3:
    case ByteCode::_iconst_4:
    case ByteCode::_iconst_5:
    	{
    		int val = code - ByteCode::_iconst_0;
			*_codeStrStream << "       #" << ByteCode::_name[code] << "\n";
			*_codeStrStream << "       movq $" << val << ", %" << getReg() << "\n";
			pushVirtualStack();
        }
        break;

    case ByteCode::_lconst_0:{
          notImplemented(code);
        }
        break;

    case ByteCode::_lconst_1:{
          notImplemented(code);
        }
        break;

    case ByteCode::_fconst_0:{
          notImplemented(code);
        }
        break;

    case ByteCode::_fconst_1:{
          notImplemented(code);
        }
        break;

    case ByteCode::_fconst_2:{
          notImplemented(code);
        }
        break;

    case ByteCode::_dconst_0:{
          notImplemented(code);
        }
        break;

    case ByteCode::_dconst_1:{
          notImplemented(code);
        }
        break;

    case ByteCode::_bipush:{
			u1 * p = &codeArray[k+1];
			u1 arg = ClassParser::readU1(p);
			*_codeStrStream << "       #"
					<< ByteCode::_name[code] << " "<< std::dec << (int) arg <<"\n";
			*_codeStrStream << "       movq $" << (int) arg << ", %" << getReg() << "\n";
			pushVirtualStack();
        }
        break;

    case ByteCode::_sipush:{
			u1 * p = &codeArray[k+1];
			u2 arg = ClassParser::readU2(p);
			*_codeStrStream << "       #"
					<< ByteCode::_name[code] << " "<< std::dec << (int) arg <<"\n";
			*_codeStrStream << "       movq $" << (int) arg << ", %" << getReg() << "\n";
			pushVirtualStack();
        }
        break;

    case ByteCode::_ldc:{
			u1 * p = &codeArray[k+1];
			u1 arg = ClassParser::readU1(p);
			*this->_codeStrStream << "       #"
					<< ByteCode::_name[code] << " #"<< std::dec << (int) arg <<"\n";
			if (SnapJVMRuntime::isVerboseMode()) {
				printf("arg=%d\n", arg);
			}
			ConstantPoolInfoPtr info = _classClass->_constantPoolInfoArray[arg];
			if (info != NULL) { // skip entries with no info
				*this->_codeStrStream << "       movq    $0x" << std::hex
						<< (unsigned long) info->toData(_classClass)
						<< ", %rdi\n";
				//printf("         mov #0x%016lx,%%rdi\n", (unsigned long) info->toData(_classClass));
			}
        }
        break;

    case ByteCode::_ldc_w:{
          notImplemented(code);
        }
        break;

    case ByteCode::_ldc2_w:{
          notImplemented(code);
        }
        break;

    case ByteCode::_iload:{
          notImplemented(code);
        }
        break;

    case ByteCode::_lload:{
          notImplemented(code);
        }
        break;

    case ByteCode::_fload:{
          notImplemented(code);
        }
        break;

    case ByteCode::_dload:{
          notImplemented(code);
        }
        break;

    case ByteCode::_aload:{
          notImplemented(code);
        }
        break;

    case ByteCode::_iload_0:
    case ByteCode::_iload_1:
    case ByteCode::_iload_2:
    case ByteCode::_iload_3:{
			int localvar = code - ByteCode::_iload_0;
			*_codeStrStream << "       #" << ByteCode::_name[code] << "\n";
			*_codeStrStream << "       movq    -"
							  << 8 * (_stackFrameLocalVars+localvar) << "(%rbp),%" << this->getReg() << "\n";
        }
		pushVirtualStack();

        break;

    case ByteCode::_lload_0:{
          notImplemented(code);
        }
        break;

    case ByteCode::_lload_1:{
          notImplemented(code);
        }
        break;

    case ByteCode::_lload_2:{
          notImplemented(code);
        }
        break;

    case ByteCode::_lload_3:{
          notImplemented(code);
        }
        break;

    case ByteCode::_fload_0:{
          notImplemented(code);
        }
        break;

    case ByteCode::_fload_1:{
          notImplemented(code);
        }
        break;

    case ByteCode::_fload_2:{
          notImplemented(code);
        }
        break;

    case ByteCode::_fload_3:{
          notImplemented(code);
        }
        break;

    case ByteCode::_dload_0:{
          notImplemented(code);
        }
        break;

    case ByteCode::_dload_1:{
            int localvar = code - ByteCode::_dload_0;
            *_codeStrStream << "      #" << ByteCode::_name[code] << "\n";
            *_codeStrStream << "      movsd    -" << 8 * (_stackFrameLocalVars + localvar) << "(%rbp),%" << this->getReg() << "\n";
            pushVirtualStack();
        }
        break;

    case ByteCode::_dload_2:{
          notImplemented(code);
        }
        break;

    case ByteCode::_dload_3:{
          notImplemented(code);
        }
        break;

    case ByteCode::_aload_0:{
          notImplemented(code);
        }
        break;

    case ByteCode::_aload_1:{
          notImplemented(code);
        }
        break;

    case ByteCode::_aload_2:{
          notImplemented(code);
        }
        break;

    case ByteCode::_aload_3:{
          notImplemented(code);
        }
        break;

    case ByteCode::_iaload:{
          notImplemented(code);
        }
        break;

    case ByteCode::_laload:{
          notImplemented(code);
        }
        break;

    case ByteCode::_faload:{
          notImplemented(code);
        }
        break;

    case ByteCode::_daload:{
          notImplemented(code);
        }
        break;

    case ByteCode::_aaload:{
          notImplemented(code);
        }
        break;

    case ByteCode::_baload:{
          notImplemented(code);
        }
        break;

    case ByteCode::_caload:{
          notImplemented(code);
        }
        break;

    case ByteCode::_saload:{
          notImplemented(code);
        }
        break;

    case ByteCode::_istore:{
          notImplemented(code);
        }
        break;

    case ByteCode::_lstore:{
          notImplemented(code);
        }
        break;

    case ByteCode::_fstore:{
          notImplemented(code);
        }
        break;

    case ByteCode::_dstore:{
          notImplemented(code);
        }
        break;

    case ByteCode::_astore:{
          notImplemented(code);
        }
        break;

    case ByteCode::_istore_0:
    case ByteCode::_istore_1:
    case ByteCode::_istore_2:
    case ByteCode::_istore_3:
		{
			popVirtualStack();
			int localvar = code - ByteCode::_istore_0;
			*_codeStrStream << "       #" << ByteCode::_name[code] << "\n";
			*_codeStrStream << "       movq    %" << this->getReg() << "," << "-"
							  << 8 * (_stackFrameLocalVars+localvar) << "(%rbp)\n";
		}
		break;

    case ByteCode::_lstore_0:{
          notImplemented(code);
        }
        break;

    case ByteCode::_lstore_1:{
          notImplemented(code);
        }
        break;

    case ByteCode::_lstore_2:{
          notImplemented(code);
        }
        break;

    case ByteCode::_lstore_3:{
          notImplemented(code);
        }
        break;

    case ByteCode::_fstore_0:{
          notImplemented(code);
        }
        break;

    case ByteCode::_fstore_1:{
          notImplemented(code);
        }
        break;

    case ByteCode::_fstore_2:{
          notImplemented(code);
        }
        break;

    case ByteCode::_fstore_3:{
          notImplemented(code);
        }
        break;

    case ByteCode::_dstore_0:{
          notImplemented(code);
        }
        break;

    case ByteCode::_dstore_1:{
            popVirtualStack();
            int localvar = code - ByteCode::_dstore_0;
            *_codeStrStream << "    #" << ByteCode::_name[code] << "\n";
            *_codeStrStream << "      movsd    %" << this->getReg() << "," << "-" << 8 * (_stackFrameLocalVars+localvar) << "(%rbp)\n";
        }
        break;

    case ByteCode::_dstore_2:{
          notImplemented(code);
        }
        break;

    case ByteCode::_dstore_3:{
          notImplemented(code);
        }
        break;

    case ByteCode::_astore_0:{
          notImplemented(code);
        }
        break;

    case ByteCode::_astore_1:{
          notImplemented(code);
        }
        break;

    case ByteCode::_astore_2:{
          notImplemented(code);
        }
        break;

    case ByteCode::_astore_3:{
          notImplemented(code);
        }
        break;

    case ByteCode::_iastore:{
          notImplemented(code);
        }
        break;

    case ByteCode::_lastore:{
          notImplemented(code);
        }
        break;

    case ByteCode::_fastore:{
          notImplemented(code);
        }
        break;

    case ByteCode::_dastore:{
          notImplemented(code);
        }
        break;

    case ByteCode::_aastore:{
          notImplemented(code);
        }
        break;

    case ByteCode::_bastore:{
          notImplemented(code);
        }
        break;

    case ByteCode::_castore:{
          notImplemented(code);
        }
        break;

    case ByteCode::_sastore:{
          notImplemented(code);
        }
        break;

    case ByteCode::_pop:{
          notImplemented(code);
        }
        break;

    case ByteCode::_pop2:{
          notImplemented(code);
        }
        break;

    case ByteCode::_dup:{
	  const char * arg1 = this->getReg();
          popVirtualStack();
	  const char * arg2 = this->getReg();
	  pushVirtualStack();
	  pushVirtualStack();

	  *_codeStrStream << "       #" << ByteCode::_name[code] << "\n";
	  *_codeStrStream << "       movq    %" << arg2 << "," << "%" << arg1 << "\n";
        }
        break;

    case ByteCode::_dup_x1:{
          notImplemented(code);
        }
        break;

    case ByteCode::_dup_x2:{
          notImplemented(code);
        }
        break;

    case ByteCode::_dup2:{
          notImplemented(code);
        }
        break;

    case ByteCode::_dup2_x1:{
          notImplemented(code);
        }
        break;

    case ByteCode::_dup2_x2:{
          notImplemented(code);
        }
        break;

    case ByteCode::_swap:{
          notImplemented(code);
        }
        break;

    case ByteCode::_iadd:{
    		popVirtualStack();
			const char * arg1 = this->getReg();
			popVirtualStack();
			const char * arg2 = this->getReg();
			pushVirtualStack();
			*_codeStrStream << "       #" << ByteCode::_name[code] << "\n";
			*_codeStrStream << "       addq    %" << arg1 << "," << "%" << arg2 << "\n";
		}
        break;

    case ByteCode::_ladd:{
          notImplemented(code);
        }
        break;

    case ByteCode::_fadd:{
          notImplemented(code);
        }
        break;

    case ByteCode::_dadd:{
            popVirtualStack();
            const char * arg1 = this->getReg();
            popVirtualStack();
            const char * arg2 = this->getReg();
            pushVirtualStack();
            *_codeStrStream << "      #" << ByteCode::_name[code] << "\n";
            *_codeStrStream << "      addsd    %" << arg1 << "," << "%" << arg2 << "\n";
        }
        break;

    case ByteCode::_isub:{
          notImplemented(code);
        }
        break;

    case ByteCode::_lsub:{
          notImplemented(code);
        }
        break;

    case ByteCode::_fsub:{
          notImplemented(code);
        }
        break;

    case ByteCode::_dsub:{
            popVirtualStack();
            const char * arg1 = this->getReg();
            popVirtualStack();
            const char * arg2 = this->getReg();
            pushVirtualStack();
            *_codeStrStream << "      #" << ByteCode::_name[code] << "\n";
            *_codeStrStream << "      addsd    %" << arg1 << "," << "%" << arg2 << "\n";
        }
        break;

    case ByteCode::_imul:{
          notImplemented(code);
        }
        break;

    case ByteCode::_lmul:{
          notImplemented(code);
        }
        break;

    case ByteCode::_fmul:{
          notImplemented(code);
        }
        break;

    case ByteCode::_dmul:{
          notImplemented(code);
        }
        break;

    case ByteCode::_idiv:{
          notImplemented(code);
        }
        break;

    case ByteCode::_ldiv:{
          notImplemented(code);
        }
        break;

    case ByteCode::_fdiv:{
          notImplemented(code);
        }
        break;

    case ByteCode::_ddiv:{
          notImplemented(code);
        }
        break;

    case ByteCode::_irem:{
          notImplemented(code);
        }
        break;

    case ByteCode::_lrem:{
          notImplemented(code);
        }
        break;

    case ByteCode::_frem:{
          notImplemented(code);
        }
        break;

    case ByteCode::_drem:{
          notImplemented(code);
        }
        break;

    case ByteCode::_ineg:{
          notImplemented(code);
        }
        break;

    case ByteCode::_lneg:{
          notImplemented(code);
        }
        break;

    case ByteCode::_fneg:{
          notImplemented(code);
        }
        break;

    case ByteCode::_dneg:{
          notImplemented(code);
        }
        break;

    case ByteCode::_ishl:{
          notImplemented(code);
        }
        break;

    case ByteCode::_lshl:{
          notImplemented(code);
        }
        break;

    case ByteCode::_ishr:{
          notImplemented(code);
        }
        break;

    case ByteCode::_lshr:{
          notImplemented(code);
        }
        break;

    case ByteCode::_iushr:{
          notImplemented(code);
        }
        break;

    case ByteCode::_lushr:{
          notImplemented(code);
        }
        break;

    case ByteCode::_iand:{
          notImplemented(code);
        }
        break;

    case ByteCode::_land:{
          notImplemented(code);
        }
        break;

    case ByteCode::_ior:{
          notImplemented(code);
        }
        break;

    case ByteCode::_lor:{
          notImplemented(code);
        }
        break;

    case ByteCode::_ixor:{
          notImplemented(code);
        }
        break;

    case ByteCode::_lxor:{
          notImplemented(code);
        }
        break;

    case ByteCode::_iinc:{
      notImplemented(code);

      /**
       * Code generation below should be correct;
       * There might be a problem with parsing iinc
       * as the program will stop parsing the rest if encountering iinc
       *   -- Muyuan 12/6/19
       */


      /*
      u1 * p = &codeArray[k+1];
      int localvar = (int)ClassParser::readU1(p);
      int incvalue = (int)ClassParser::readU1(p);
      if(incvalue > 127) incvalue -= 256; //as incvalue is supposed to be a signed byte
      *_codeStrStream << "       #" << ByteCode::_name[code] << " " << localvar << " " << (int)incvalue <<"\n";
      const char* reg = this->getReg();

      *_codeStrStream << "       movq    -"
		      << 8 * (_stackFrameLocalVars+localvar) << "(%rbp),%" << reg << "\n";
      if(incvalue == 1){
	*_codeStrStream << "       incq    %" << reg << "\n";
      }else if(incvalue == -1){
	*_codeStrStream << "       decq    %" << reg << "\n";
      }else{
	*_codeStrStream << "       addq    $" << incvalue << "," << "%" << reg << "\n";
      }

      *_codeStrStream << "       movq    %" << reg << "," << "-"
							  << 8 * (_stackFrameLocalVars+localvar) << "(%rbp)\n";

      */
        }
        break;

    case ByteCode::_i2l:{
          notImplemented(code);
        }
        break;

    case ByteCode::_i2f:{
          notImplemented(code);
        }
        break;

    case ByteCode::_i2d:{
          notImplemented(code);
        }
        break;

    case ByteCode::_l2i:{
          notImplemented(code);
        }
        break;

    case ByteCode::_l2f:{
          notImplemented(code);
        }
        break;

    case ByteCode::_l2d:{
          notImplemented(code);
        }
        break;

    case ByteCode::_f2i:{
          notImplemented(code);
        }
        break;

    case ByteCode::_f2l:{
          notImplemented(code);
        }
        break;

    case ByteCode::_f2d:{
          notImplemented(code);
        }
        break;

    case ByteCode::_d2i:{
          notImplemented(code);
        }
        break;

    case ByteCode::_d2l:{
          notImplemented(code);
        }
        break;

    case ByteCode::_d2f:{
          notImplemented(code);
        }
        break;

    case ByteCode::_i2b:{
          notImplemented(code);
        }
        break;

    case ByteCode::_i2c:{
          notImplemented(code);
        }
        break;

    case ByteCode::_i2s:{
          notImplemented(code);
        }
        break;

    case ByteCode::_lcmp:{
          notImplemented(code);
        }
        break;

    case ByteCode::_fcmpl:{
          notImplemented(code);
        }
        break;

    case ByteCode::_fcmpg:{
          notImplemented(code);
        }
        break;

    case ByteCode::_dcmpl:{
          notImplemented(code);
        }
        break;

    case ByteCode::_dcmpg:{
          notImplemented(code);
        }
        break;

    case ByteCode::_ifeq:
    case ByteCode::_ifne:
    case ByteCode::_iflt:
    case ByteCode::_ifge:
    case ByteCode::_ifgt:
    case ByteCode::_ifle:
    case ByteCode::_if_icmpeq:
    case ByteCode::_if_icmpne:
    case ByteCode::_if_icmplt:
    case ByteCode::_if_icmpge:
    case ByteCode::_if_icmpgt:
    case ByteCode::_if_icmple:{
			int cmp_to_zero = code < ByteCode::_if_icmpeq ? 1 : 0;
			int offset = code - (cmp_to_zero ? ByteCode::_ifeq : ByteCode::_if_icmpeq);
			std::string cmpflag = "";
			if(offset == 0){
				cmpflag = "e";
			}else if(offset == 1){
				cmpflag = "ne";
			}else if(offset == 2){
				cmpflag = "l";
			}else if(offset == 3){
				cmpflag = "ge";
			}else if(offset == 4){
				cmpflag = "g";
			}else if(offset == 5){
				cmpflag = "le";
			}
			//compare the twop two values on the operand stack; if succeeds, jump to the address
			u1 * p = &codeArray[k+1];
			u2 arg = ClassParser::readU2(p) + k;
			*this->_codeStrStream << "       #"
					<< ByteCode::_name[code] << " "<< std::dec << (int) arg <<"\n";

			//compare the top two numbers in the operand stack
			popVirtualStack();
			const char* reg1 = getReg();
			if(cmp_to_zero){
				//$0 has to be the first operand for GAS/AT&T syntax
				*this->_codeStrStream << "       cmpq $0, %" << reg1 << "\n";
			}else{
				popVirtualStack();
				*this->_codeStrStream << "       cmpq %" << reg1 << ", %" << getReg() <<"\n";
			}

			*this->_codeStrStream << "       j" << cmpflag << " offset_" << std::dec << arg << "\n";
    }
      break;


    case ByteCode::_if_acmpeq:{
    	  notImplemented(code);
        }
        break;

    case ByteCode::_if_acmpne:{
    	  notImplemented(code);
        }
        break;

    case ByteCode::_goto:{
			u1 * p = &codeArray[k+1];
			u2 arg = ClassParser::readU2(p) + k;
			*this->_codeStrStream << "       #"
					<< ByteCode::_name[code] << " "<< std::dec << (int) arg <<"\n";
			*this->_codeStrStream << "       jmp offset_" << std::dec << arg << "\n";
        }
        break;

    case ByteCode::_jsr:{
          notImplemented(code);
        }
        break;

    case ByteCode::_ret:{
    	  notImplemented(code);
        }
        break;

    case ByteCode::_tableswitch:{
          notImplemented(code);
        }
        break;

    case ByteCode::_lookupswitch:{
          notImplemented(code);
        }
        break;

    case ByteCode::_ireturn:{
          notImplemented(code);
        }
        break;

    case ByteCode::_lreturn:{
          notImplemented(code);
        }
        break;

    case ByteCode::_freturn:{
          notImplemented(code);
        }
        break;

    case ByteCode::_dreturn:{
          notImplemented(code);
        }
        break;

    case ByteCode::_areturn:{
          notImplemented(code);
        }
        break;

    case ByteCode::_return:{
    	restoreRegsBeforeReturn();
		*_codeStrStream << "       # " << ByteCode::_name[code] << "\n";
		*_codeStrStream << "       movq    %rbp, %rsp\n";
		*_codeStrStream << "       leave\n";
		*_codeStrStream << "       ret\n";
        }
        break;

    case ByteCode::_getstatic:{
		*_codeStrStream << "       #" << ByteCode::_name[code] << "\n";
		u1 * p = &codeArray[k+1];
		u2 index = (u2) ClassParser::readU1() << 8 | (u2) ClassParser::readU1();
		CONSTANT_Fieldref_info fieldref_info = _classClass->_ConstantPoolInfo[index];
		CONSTANT_String_info nameStringInfo = _classClass->_constantPoolInfoArray[fieldref_info.name_and_type_index];
		u8 value = _classClass->getField(nameStringInfo.toData(_classClass));
		*_codeStrStream << "	   movq	   $" << value << ", %" << getReg() << "\n";
			pushVirtualStack();
    }
        break;
	//Does not implement all of the extra type checking mentioned in JVMS since we use only 8B
    case ByteCode::_putstatic:{
		*_codeStrStream << "       #" << ByteCode::_name[code] << "\n";
		u1 * p = &codeArray[k+1];
		u2 index = (u2) ClassParser::readU1() << 8 | (u2) ClassParser::readU1();
		CONSTANT_Fieldref_info fieldref_info = _classClass->_ConstantPoolInfo[index];
		CONSTANT_String_info nameStringInfo = _classClass->_constantPoolInfoArray[fieldref_info.name_and_type_index];
		u8 value = _classClass->getField(nameStringInfo.toData(_classClass));
		*_codeStrStream << "	   movq	   $" << value << ", %" << getReg() << "\n";
		pushVirtualStack();
	}
        break;

    case ByteCode::_getfield:{
          notImplemented(code);
        }
        break;

    case ByteCode::_putfield:{
          notImplemented(code);
        }
        break;

    case ByteCode::_invokevirtual:{
    	    *this->_codeStrStream << "";
			*this->_codeStrStream << "       #"<< ByteCode::_name[code] <<"\n";
			*this->_codeStrStream << "       mov    $0x" << std::hex
		  						  << (unsigned long) simplePrintf << ",%rax" << "\n";
			*this->_codeStrStream << "       call   *%rax\n";
			//notImplemented(code);
        }
        break;

    case ByteCode::_invokespecial:{
          notImplemented(code);
        }
        break;

    case ByteCode::_invokestatic:{
          notImplemented(code);
        }
        break;

    case ByteCode::_invokeinterface:{
          notImplemented(code);
        }
        break;

    case ByteCode::_invokedynamic:{
          notImplemented(code);
        }
        break;

    case ByteCode::_new:{
          notImplemented(code);
        }
        break;

    case ByteCode::_newarray:{
          notImplemented(code);
        }
        break;

    case ByteCode::_anewarray:{
          notImplemented(code);
        }
        break;

    case ByteCode::_arraylength:{
          notImplemented(code);
        }
        break;

    case ByteCode::_athrow:{
          notImplemented(code);
        }
        break;

    case ByteCode::_checkcast:{
          notImplemented(code);
        }
        break;

    case ByteCode::_instanceof:{
          notImplemented(code);
        }
        break;

    case ByteCode::_monitorenter:{
          notImplemented(code);
        }
        break;

    case ByteCode::_monitorexit:{
          notImplemented(code);
        }
        break;

    case ByteCode::_wide:{
          notImplemented(code);
        }
        break;

    case ByteCode::_multianewarray:{
          notImplemented(code);
        }
        break;

    case ByteCode::_ifnull:{
          notImplemented(code);
        }
        break;

    case ByteCode::_ifnonnull:{
          notImplemented(code);
        }
        break;

    case ByteCode::_goto_w:{
			u1 * p = &codeArray[k+1];
			u4 arg = ClassParser::readU4(p) + k;
			*this->_codeStrStream << "       #"
					<< ByteCode::_name[code] << " "<< std::dec << (int) arg <<"\n";
			*this->_codeStrStream << "       jmp offset_" << std::dec << arg << "\n";
        }
        break;

    case ByteCode::_jsr_w:{
          notImplemented(code);
        }
        break;

    case ByteCode::_breakpoint:{
          notImplemented(code);
        }
        break;

    default:
    	  notImplemented(code);
        break;
	}
}

void
ByteCodeGen_X86_64::notImplemented(ByteCode::Code code)
{
	*this->_codeStrStream << "       # Not implemented. "
						<< ByteCode::_name[code] << "\n";
	if (SnapJVMRuntime::isVerboseMode()) {
		printf("ByteCodeGen_X86_64:Not implemented \"%s\"\n", ByteCode::_name[code]);
	}
}




