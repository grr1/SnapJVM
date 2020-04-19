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

ByteCodeGen_X86_64::ByteCodeGen_X86_64(ClassClass *classClass) {
    _classClass = classClass;
    _assembler_X86_64 = new Assembler_X86_64();
    _top = 0;

    _stackRegs[0] = "rbx";
    _stackRegs[1] = "r12";
    _stackRegs[2] = "r13";
    _stackRegs[3] = "r14";
    _stackRegs[4] = "r15";
    _maxStackRegs = 5;

    // rdi - 1st argument - SnapJVMRuntime::TheJVMRuntime()
    // rsi - 2nd argument - Object * o
    // rdx - 3rd argument - 1st method argument
    // rcx - 4th argument - 2nd method argument
    // r8  - 5th argument - 3rd method argument
    // r9  - 6th argument - 4th method argument

    _argumentRegs[0] = "rdi";
    _argumentRegs[1] = "rsi";
    _argumentRegs[2] = "rdx";
    _argumentRegs[3] = "rcx";
    _argumentRegs[4] = "r8";
    _argumentRegs[5] = "r9";
    _maxArgumentRegs = 6;
}

ByteCodeGen_X86_64::~ByteCodeGen_X86_64() {

}

void
ByteCodeGen_X86_64::pushVirtualStack() {
    _top++;
    if (_top >= _maxStackRegs) {
        // Save register
        *this->_codeStrStream << "       movq   %" << getReg() << "," << "-"
                              << 8 * (_stackFRameJavaStack + _top) << "(%rbp)       # save top=" << _top << "\n";
    }
}

void
ByteCodeGen_X86_64::popVirtualStack() {
    _top--;
    if (_top >= _maxStackRegs) {
        // Save register
        *this->_codeStrStream << "       movq   "
                              << "-" << 8 * (_stackFRameJavaStack + _top) << "(%rbp) ,%" << getReg()
                              << "       # save top=" << _top << "\n";
    }

    if (_top < 0) {
        printf("********************** ERROR *******************\nTop=%d\n", _top);
        printf("Code: %s\n", this->_codeStrStream->str().c_str());
    }
}

const char *
ByteCodeGen_X86_64::getReg() {
    return _stackRegs[_top % _maxStackRegs];
}

void
ByteCodeGen_X86_64::codeGen() {
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

        MethodInfo *methodInfo = _classClass->_methodInfoArray[i];

        if (SnapJVMRuntime::isVerboseMode()) {
            // Print type
            printf("Type name:");
            _classClass->printExtendedType(methodInfo->descriptor_index);
            printf("Method Name:");
            _classClass->printStringDecorated(methodInfo->name_index);
        }

        ConstantPoolInfo *info = _classClass->_constantPoolInfoArray[methodInfo->name_index];

        CONSTANT_Utf8_info *uinfo = dynamic_cast<CONSTANT_Utf8_info *>(info);
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
            AttributeInfo *attrInfo = methodInfo->attributesArray[j];
            if (SnapJVMRuntime::isVerboseMode()) {
                printf("    %d:", j);
                _classClass->printStringDecorated(attrInfo->attribute_name_index);
                printf("\n");
            }
            //printf("    attributes_count=%d\n", attrInfo->attribute_length);
            //printf("\n");
            ConstantPoolInfo *info = _classClass->_constantPoolInfoArray[attrInfo->attribute_name_index];
            CONSTANT_Utf8_info *uinfo = dynamic_cast<CONSTANT_Utf8_info *>(info);
            const char *sname = "";
            if (uinfo != NULL) {
                sname = (const char *) uinfo->bytesArray;
            }
            if (!strcmp(sname, "Code")) {
                //CodeAttribute * codeAttribute = ()
                unsigned char *uptr = attrInfo->infoArray;
                _max_stack = _classParser->readU2(uptr);
                _max_locals = _classParser->readU2(uptr);
                _code_length = _classParser->readU4(uptr);

                _method->_maxStack = _max_stack;
                _method->_maxLocals = _max_locals;

                u2 max_args = 6;
                _method->_maxArgs = max_args;

                u1 *codeArray = uptr;
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

                //align stack pointer to 16 bytes
                if (_stackFrameEnd % 2 != 0) {
                    _stackFrameEnd += 1;
                }

                *_codeStrStream << "\n";
                *_codeStrStream << _method->_methodName << ":\n";
                *_codeStrStream << "       #max_stack=" << _max_stack
                                << "       max_locals=" << _max_locals << "\n\n";
                *_codeStrStream << "       # Save frame pointer\n";
                *_codeStrStream << "       pushq   %rbp\n";
                *_codeStrStream << "       movq    %rsp, %rbp\n\n";
                *_codeStrStream << "       # Allocate space for locals and stack\n";
                *_codeStrStream << "       subq    $" << std::dec << 8 * (_stackFrameEnd) << ",%rsp\n";

                // Save argument registers
                *_codeStrStream << "       # Save arguments\n";
                for (int i = 0; i < _maxArgumentRegs; i++) {
                    *_codeStrStream << "       mov   " << _argumentRegs[i] << ",-" << 8 * (i + _stackFrameArgs)
                                    << "(%rbp)\n";
                }
                *_codeStrStream << "\n";

                // Save callee saved registers
                *_codeStrStream << "\n";
                *_codeStrStream << "       #Save callee saved registers\n";
                for (int i = 0; i < _maxStackRegs; i++) {
                    *_codeStrStream << "       mov   " << _stackRegs[i] << ",-" << 8 * (i + _stackFrameCalleeSaved)
                                    << "(%rbp)\n";
                }
                *_codeStrStream << "\n";
                while (k < _code_length) {
                    *_codeStrStream << "offset_" << std::dec << k << ":\n";
                    ByteCode::Code c = (ByteCode::Code) codeArray[k];

                    if (SnapJVMRuntime::isVerboseMode()) {
                        printf("         %d: %s ", k, ByteCode::_name[c]);
                        u2 flags = ByteCode::_flags[c];
                        if (flags & ByteCode::Flags::_fmt_has_j &&
                            flags & ByteCode::Flags::_fmt_has_u2) {
                            u1 *p = uptr + 1;
                            u2 arg = _classParser->readU2(p);
                            printf(" #%d", arg);
                            ConstantPoolInfoPtr info = _classClass->_constantPoolInfoArray[arg];
                            if (info != NULL) { // skip entries with no info
                                printf("\t// ");
                                info->printShort(_classClass);
                            }
                        } else if (flags & ByteCode::Flags::_fmt_has_k &&
                                   flags & ByteCode::Flags::_fmt_has_u2) {
                            u1 *p = uptr + 1;
                            u2 arg = _classParser->readU2(p);
                            printf(" #%d", arg);
                            ConstantPoolInfoPtr info = _classClass->_constantPoolInfoArray[arg];
                            if (info != NULL) { // skip entries with no info
                                printf("\t// ");
                                info->printShort(_classClass);
                            }
                        } else if (flags & ByteCode::Flags::_fmt_has_k &&
                                   !(flags & ByteCode::Flags::_fmt_has_u2)) {
                            u1 *p = uptr + 1;
                            u1 arg = _classParser->readU1(p);
                            printf(" #%d", arg);
                            ConstantPoolInfoPtr info = _classClass->_constantPoolInfoArray[arg];
                            if (info != NULL) { // skip entries with no info
                                printf("\t// ");
                                info->printShort(_classClass);
                            }
                        }
                        printf("\n");
                    }

                    //printFlags(flags);
                    //printf("         %d: %d\n",k, attrInfo->infoArray[k]);
                    //k++;

                    // *_codeStrStream << "_maxArgumentRegs=" << _maxArgumentRegs << "\n";

                    // Generate code for every ByteCode
                    codeGenOne(c, codeArray, k);

                    int byteCodeLength = ByteCode::_lengths[c];
                    //calculate the length of variable-length ByteCode
                    //similar to Code printing in method print() in ClassClass.cpp
                    if (c == ByteCode::_lookupswitch) {
                        int padBytes = 3 - k % 4;
                        //npairs starts at k+1+padBytes + 4;
                        u1 *p = &codeArray[k + padBytes + 5];
                        int nPairs = (int) ClassParser::readU4(p);
                        byteCodeLength = padBytes + 9 + nPairs * 8;
                    } else if (c == ByteCode::_tableswitch) {
                        int padBytes = 3 - k % 4;
                        //lowbyte starts at k+1+padBytes + 4;
                        u1 *p = &codeArray[k + padBytes + 5];
                        int lowValue = (int) ClassParser::readU4(p);
                        //highbyte starts at k+1+padBytes + 8;
                        p = &codeArray[k + padBytes + 9];
                        int highValue = (int) ClassParser::readU4(p);
                        byteCodeLength = padBytes + 13 + (highValue - lowValue + 1) * 4;
                    }
                    k += ByteCode::_lengths[c] & 0xF;
                    uptr += ByteCode::_lengths[c] & 0xF;
                }

                // Done by return
                // *_codeStrStream <<
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
ByteCodeGen_X86_64::restoreRegsBeforeReturn() {
    // *_codeStrStream << "_maxArgumentRegs=" << _maxArgumentRegs << "\n";

    // Restore callee saved registers
    *_codeStrStream << "\n";
    *_codeStrStream << "       #Restore callee saved registers\n";
    for (int i = 0; i < _maxStackRegs; i++) {
        *_codeStrStream << "       mov   -" << std::dec << 8 * (i + _stackFrameCalleeSaved) << "(%rbp), %"
                        << _stackRegs[i] << "\n";
    }
    *_codeStrStream << "\n";
}

void
simplePrintf(const char *s) {
    printf("%s\n", s);
}

void
simplePrintfHex(u8 l) {
    printf("0x%08x\n",  *(int *) &l);
}

void simplePrintfD(u8 d) {
    printf("%lf\n", *(double *) &d);
}


int doubletest = 0;


void ByteCodeGen_X86_64::codeGenOne(ByteCode::Code code, u1 *codeArray, int k) {
    switch (code) {
        case ByteCode::_nop: {
            notImplemented(code);
        }
            break;

        case ByteCode::_aconst_null: {
            notImplemented(code);
        }
            break;

        case ByteCode::_iconst_m1: // push -1
        case ByteCode::_iconst_0:
        case ByteCode::_iconst_1:
        case ByteCode::_iconst_2:
        case ByteCode::_iconst_3:
        case ByteCode::_iconst_4:
        case ByteCode::_iconst_5: {
            int val = code - ByteCode::_iconst_0;
            *_codeStrStream << "       #" << ByteCode::_name[code] << "\n";
            *_codeStrStream << "       movq $" << val << ", %" << getReg() << "\n";
            pushVirtualStack();
        }
            break;

        case ByteCode::_lconst_0: {
            notImplemented(code);
        }
            break;

        case ByteCode::_lconst_1: {
            notImplemented(code);
        }
            break;

        case ByteCode::_fconst_0: {
            double d = 0;
            u8 *ds = (u8*) &d;
            *_codeStrStream << "      #" << ByteCode::_name[code] << "\n";
            *_codeStrStream << "        movq $0x" << std::hex << *ds << ", %" << getReg() << "\n";
            pushVirtualStack();
        }
            break;

        case ByteCode::_fconst_1: {
            double d = 1;
            u8 *ds = (u8*) &d;
            *_codeStrStream << "      #" << ByteCode::_name[code] << "\n";
            *_codeStrStream << "        movq $0x" << std::hex << *ds << ", %" << getReg() << "\n";
            pushVirtualStack();
        }
            break;

        case ByteCode::_fconst_2: {
            double d = 2;
            u8 *ds = (u8*) &d;
            *_codeStrStream << "      #" << ByteCode::_name[code] << "\n";
            *_codeStrStream << "        movq $0x" << std::hex << *ds << ", %" << getReg() << "\n";
            pushVirtualStack();
        }
            break;

        case ByteCode::_dconst_0: {
            double d = 0;
            u8 *ds = (u8*) &d;
            *_codeStrStream << "      #" << ByteCode::_name[code] << "\n";
            *_codeStrStream << "        movq $0x" << std::hex << *ds << ", %" << getReg() << "\n";
            pushVirtualStack();
        }
            break;

        case ByteCode::_dconst_1: {
            double d = 1;
            u8 *ds = (u8*) &d;
            *_codeStrStream << "      #" << ByteCode::_name[code] << "\n";
            *_codeStrStream << "        movq $0x" << std::hex <<*ds << ", %" << getReg() << "\n";
            pushVirtualStack();
        }
            break;

        case ByteCode::_bipush: {
            u1 *p = &codeArray[k + 1];
            signed char arg = (signed char) ClassParser::readU1(p);
            *_codeStrStream << "       #"
                            << ByteCode::_name[code] << " " << std::dec << (int) arg << "\n";
            *_codeStrStream << "       movq $" << (int) arg << ", %" << getReg() << "\n";
            pushVirtualStack();
        }
            break;

        case ByteCode::_sipush: {
            u1 *p = &codeArray[k + 1];
            short arg = (short) ClassParser::readU2(p);
            *_codeStrStream << "       #"
                            << ByteCode::_name[code] << " " << std::dec << (int) arg << "\n";
            *_codeStrStream << "       movq $" << (int) arg << ", %" << getReg() << "\n";
            pushVirtualStack();
        }
            break;

        case ByteCode::_ldc: {
            u1 *p = &codeArray[k + 1];
            u1 arg = ClassParser::readU1(p);
            *this->_codeStrStream << "       #"
                                  << ByteCode::_name[code] << " #" << std::dec << (int) arg << "\n";
            if (SnapJVMRuntime::isVerboseMode()) {
                printf("arg=%d\n", arg);
            }
            ConstantPoolInfoPtr info = _classClass->_constantPoolInfoArray[arg];
            if (info != NULL) { // skip entries with no info
                *this->_codeStrStream << "       movq    $0x" << std::hex
                                      << (unsigned long) info->toData(_classClass)
                                      << ", %" << getReg() << "\n";
                pushVirtualStack();
            }
        }
            break;

        case ByteCode::_ldc_w: {
            u1 *p1 = &codeArray[k + 1];
            u1 arg1 = ClassParser::readU1(p1);
            u1 *p2 = &codeArray[k + 2];
            u1 arg2 = ClassParser::readU1(p2);
            int arg = ((int) arg1) << 8;
            arg = arg | ((int) arg2);
            *this->_codeStrStream << "       #" << ByteCode::_name[code] << " #" << arg << "\n";
            if (SnapJVMRuntime::isVerboseMode()) {
                printf("arg=%d\n", arg);
            }
            ConstantPoolInfoPtr info = _classClass->_constantPoolInfoArray[arg];
            CONSTANT_Double_info *dinfo = (CONSTANT_Double_info *) info;

            if (info != NULL) {
                u8 *doubleAsLong = (u8 *) &(dinfo->value);
                *this->_codeStrStream << "       movq   $0x" << std::hex << *doubleAsLong << ", " << this->getReg()
                                      << "\n";
            }
            pushVirtualStack();
        }
            break;

        case ByteCode::_ldc2_w: {
            u1 * p1 = &codeArray[k+1];
            u1 arg1 = ClassParser::readU1(p1);
            u1 * p2 = &codeArray[k+2];
            u1 arg2 = ClassParser::readU1(p2);
            int arg = ((int)arg1) << 8;
            arg = arg | ((int)arg2);
            *this->_codeStrStream << "       #" << ByteCode::_name[code] << " #" << arg << "\n";
            if (SnapJVMRuntime::isVerboseMode()){
                printf("arg=%d\n",arg);
            }
            ConstantPoolInfoPtr info = _classClass->_constantPoolInfoArray[arg];
            CONSTANT_Double_info *dinfo = (CONSTANT_Double_info*) info;

            if (info != NULL){
                u8 *doubleAsLong = (u8*) &(dinfo->value);
                //*this->_codeStrStream << "       movq   $" << dinfo->value << ", %rdi" << "\n";
                *this->_codeStrStream << "       movq   $0x" << std::hex << *doubleAsLong << ", %" << getReg() << "\n";
            }
            pushVirtualStack();
        }
            break;

        case ByteCode::_iload:
        case ByteCode::_aload: {
            u1 *p = &codeArray[k + 1];
            u1 arg = ClassParser::readU1(p);
            *_codeStrStream << "       #" << ByteCode::_name[code] << "\n";
            *_codeStrStream << "       movq    -"
                            << 8 * (_stackFrameLocalVars + arg) << "(%rbp),%" << this->getReg() << "\n";
        }
            pushVirtualStack();

            break;

        case ByteCode::_lload: {
            notImplemented(code);
        }
            break;

        case ByteCode::_fload: {
            notImplemented(code);
        }
            break;

        case ByteCode::_dload: {
            u1 *p = &codeArray[k + 1];
            u1 arg = ClassParser::readU1(p);
            int localvar = (int) arg;
            *_codeStrStream << "      #" << ByteCode::_name[code] << "\n";
            *_codeStrStream << "      movq    -" << 8 * (_stackFrameLocalVars + localvar) << "(%rbp),%"
                            << this->getReg() << "\n";
            pushVirtualStack();
        }
            break;

        case ByteCode::_iload_0:
        case ByteCode::_iload_1:
        case ByteCode::_iload_2:
        case ByteCode::_iload_3:
        case ByteCode::_aload_0:
        case ByteCode::_aload_1:
        case ByteCode::_aload_2:
        case ByteCode::_aload_3: 	{
	  int base_code = ByteCode::_iload_0;
	  if(code >= ByteCode::_aload_0){
	    base_code = ByteCode::_aload_0;
	  }
            int localvar = code - base_code;
            *_codeStrStream << "       #" << ByteCode::_name[code] << "\n";
            *_codeStrStream << "       movq    -"
                            << 8 * (_stackFrameLocalVars + localvar) << "(%rbp),%" << this->getReg() << "\n";
        }
            pushVirtualStack();

            break;

        case ByteCode::_lload_0: {
            notImplemented(code);
        }
            break;

        case ByteCode::_lload_1: {
            notImplemented(code);
        }
            break;

        case ByteCode::_lload_2: {
            notImplemented(code);
        }
            break;

        case ByteCode::_lload_3: {
            notImplemented(code);
        }
            break;

        case ByteCode::_fload_0: {
            notImplemented(code);
        }
            break;

        case ByteCode::_fload_1: {
            notImplemented(code);
        }
            break;

        case ByteCode::_fload_2: {
            notImplemented(code);
        }
            break;

        case ByteCode::_fload_3: {
            notImplemented(code);
        }
            break;

        case ByteCode::_dload_0: {
            int localvar = code - ByteCode::_dload_0;
            *_codeStrStream << "      #" << ByteCode::_name[code] << "\n";
            *_codeStrStream << "      movq    -" << 8 * (_stackFrameLocalVars + localvar) << "(%rbp),%"
                            << this->getReg() << "\n";
            pushVirtualStack();
        }
            break;

        case ByteCode::_dload_1: {
            int localvar = code - ByteCode::_dload_0;
            *_codeStrStream << "      #" << ByteCode::_name[code] << "\n";
            *_codeStrStream << "      movq    -" << 8 * (_stackFrameLocalVars + localvar) << "(%rbp),%"
                            << this->getReg() << "\n";
            pushVirtualStack();
        }
            break;

        case ByteCode::_dload_2: {
            int localvar = code - ByteCode::_dload_0;
            *_codeStrStream << "      #" << ByteCode::_name[code] << "\n";
            *_codeStrStream << "      movq    -" << 8 * (_stackFrameLocalVars + localvar) << "(%rbp),%"
                            << this->getReg() << "\n";
            pushVirtualStack();
        }
            break;

        case ByteCode::_dload_3: {
            int localvar = code - ByteCode::_dload_0;
            *_codeStrStream << "      #" << ByteCode::_name[code] << "\n";
            *_codeStrStream << "      movq    -" << 8 * (_stackFrameLocalVars + localvar) << "(%rbp),%"
                            << this->getReg() << "\n";
            pushVirtualStack();
        }
            break;

        case ByteCode::_iaload:
        case ByteCode::_laload:
        case ByteCode::_faload:
        case ByteCode::_daload:
        case ByteCode::_baload: 
        case ByteCode::_caload: 
        case ByteCode::_saload: {
	  BasicType type = T_INT;
	  switch(code){
	  case ByteCode::_iaload:
	    type = T_INT;
	    break;
	  case ByteCode::_laload:
	    type = T_LONG;
	    break;
	  case ByteCode::_faload:
	    type = T_FLOAT;
	    break;
	  case ByteCode::_daload:
	    type = T_DOUBLE;
	    break;
	  case ByteCode::_baload:
	    type = T_BYTE; //or boolean
	    break;
	  case ByteCode::_caload:
	    type = T_CHAR;
	    break;
	  case ByteCode::_saload:
	    type = T_SHORT;
	    break;
	  }
	  *_codeStrStream << "       #" << ByteCode::_name[code] << "\n";
	  //type: first parameter (rdi)
	  *_codeStrStream << "       mov    $" << std::dec << type << ", %rdi\n";
	  //index: second parameter (rsi)
	  popVirtualStack();
	  *_codeStrStream << "       mov    %" << getReg() << ", %rsi\n";
	  //arrayref: third parameter (rdx)
	  popVirtualStack();
	  *_codeStrStream << "       mov    %" << getReg() << ", %rdx\n";
	  //calls runtime_array_load
	  *_codeStrStream << "       mov    $0x" << std::hex
				<< (unsigned long) SnapJVMRuntime::runtime_array_load << ", %rax" << "\n";
	  *_codeStrStream << "       call   *%rax\n";
	  //push returning value back onto virtual stack
	  *_codeStrStream << "       mov    %rax, %" << getReg() << "\n";
	  pushVirtualStack();
        }
            break;

        case ByteCode::_aaload: {
            notImplemented(code);
        }
            break;
	    
        case ByteCode::_istore:
        case ByteCode::_astore: {
            u1 *p = &codeArray[k + 1];
            u1 arg = ClassParser::readU1(p);
            *_codeStrStream << "       #" << ByteCode::_name[code] << "\n";
            *_codeStrStream << "       movq    %" << this->getReg() << ","
                            << "-" << 8 * (_stackFrameLocalVars + arg) << "(%rbp)\n";
        }
            break;

        case ByteCode::_lstore: {
            notImplemented(code);
        }
            break;

        case ByteCode::_fstore: {
            notImplemented(code);
        }
            break;

        case ByteCode::_dstore: {
            popVirtualStack();
            u1 *p = &codeArray[k + 1];
            u1 arg = ClassParser::readU1(p);
            int localvar = (int) arg;
            *_codeStrStream << "    #" << ByteCode::_name[code] << "\n";
            *_codeStrStream << "      movq    %" << this->getReg() << "," << "-"
                            << 8 * (_stackFrameLocalVars + localvar) << "(%rbp)\n";
        }
            break;

        case ByteCode::_istore_0:
        case ByteCode::_istore_1:
        case ByteCode::_istore_2:
        case ByteCode::_istore_3:
        case ByteCode::_astore_0:
        case ByteCode::_astore_1:
        case ByteCode::_astore_2:
        case ByteCode::_astore_3: {
            popVirtualStack();
	    
	    int base_code = ByteCode::_istore_0;
	    if(code >= ByteCode::_astore_0){
	      base_code = ByteCode::_astore_0;
	    }
	  
            int localvar = code - base_code;
            *_codeStrStream << "       #" << ByteCode::_name[code] << "\n";
            *_codeStrStream << "       movq    %" << this->getReg() << "," << "-"
                            << 8 * (_stackFrameLocalVars + localvar) << "(%rbp)\n";
        }
            break;

        case ByteCode::_lstore_0: {
            notImplemented(code);
        }
            break;

        case ByteCode::_lstore_1: {
            notImplemented(code);
        }
            break;

        case ByteCode::_lstore_2: {
            notImplemented(code);
        }
            break;

        case ByteCode::_lstore_3: {
            notImplemented(code);
        }
            break;

        case ByteCode::_fstore_0: {
            notImplemented(code);
        }
            break;

        case ByteCode::_fstore_1: {
            notImplemented(code);
        }
            break;

        case ByteCode::_fstore_2: {
            notImplemented(code);
        }
            break;

        case ByteCode::_fstore_3: {
            notImplemented(code);
        }
            break;

        case ByteCode::_dstore_0: {
            popVirtualStack();
            int localvar = code - ByteCode::_dstore_0;
            *_codeStrStream << "    #" << ByteCode::_name[code] << "\n";
            *_codeStrStream << "      movq    %" << this->getReg() << "," << "-"
                            << 8 * (_stackFrameLocalVars + localvar) << "(%rbp)\n";
        }
            break;

        case ByteCode::_dstore_1: {
            popVirtualStack();
            int localvar = code - ByteCode::_dstore_0;
            *_codeStrStream << "    #" << ByteCode::_name[code] << "\n";
            *_codeStrStream << "      movq    %" << this->getReg() << "," << "-"
                            << 8 * (_stackFrameLocalVars + localvar) << "(%rbp)\n";
        }
            break;

        case ByteCode::_dstore_2: {
            popVirtualStack();
            int localvar = code - ByteCode::_dstore_0;
            *_codeStrStream << "    #" << ByteCode::_name[code] << "\n";
            *_codeStrStream << "      movq    %" << this->getReg() << "," << "-"
                            << 8 * (_stackFrameLocalVars + localvar) << "(%rbp)\n";
        }
            break;

        case ByteCode::_dstore_3: {
            popVirtualStack();
            int localvar = code - ByteCode::_dstore_0;
            *_codeStrStream << "    #" << ByteCode::_name[code] << "\n";
            *_codeStrStream << "      movq    %" << this->getReg() << "," << "-"
                            << 8 * (_stackFrameLocalVars + localvar) << "(%rbp)\n";
        }
            break;

        case ByteCode::_iastore:
        case ByteCode::_lastore:
        case ByteCode::_fastore:
        case ByteCode::_dastore:
        case ByteCode::_bastore:
        case ByteCode::_castore:
        case ByteCode::_sastore: {
	  BasicType type = T_INT;
	  switch(code){
	  case ByteCode::_iastore:
	    type = T_INT;
	    break;
	  case ByteCode::_lastore:
	    type = T_LONG;
	    break;
	  case ByteCode::_fastore:
	    type = T_FLOAT;
	    break;
	  case ByteCode::_dastore:
	    type = T_DOUBLE;
	    break;
	  case ByteCode::_bastore:
	    type = T_BYTE; //or boolean
	    break;
	  case ByteCode::_castore:
	    type = T_CHAR;
	    break;
	  case ByteCode::_sastore:
	    type = T_SHORT;
	    break;
	  }
	  *_codeStrStream << "       #" << ByteCode::_name[code] << "\n";
	  //type: first parameter (rdi)
	  *_codeStrStream << "       movq    $" << std::dec << type << ", %rdi\n";
	  //value: second parameter (rsi)
	  popVirtualStack();
	  *_codeStrStream << "       movq    %" << getReg() << ", %rsi\n";
	  //index: third parameter (rdx)
	  popVirtualStack();
	  *_codeStrStream << "       movq    %" << getReg() << ", %rdx\n";
	  //arrayref: fourth parameter (rcx)
	  popVirtualStack();
	  *_codeStrStream << "       movq    %" << getReg() << ", %rcx\n";
	  //calls runtime_array_store
	  *_codeStrStream << "       movq    $0x" << std::hex
				<< (unsigned long) SnapJVMRuntime::runtime_array_store << ", %rax" << "\n";
	  *_codeStrStream << "       call   *%rax\n";
        }
            break;

        case ByteCode::_aastore: {
            notImplemented(code);
        }
            break;
	    
        case ByteCode::_pop: {
            notImplemented(code);
        }
            break;

        case ByteCode::_pop2: {
            notImplemented(code);
        }
            break;

        case ByteCode::_dup: {	    
            popVirtualStack();
            const char *arg = this->getReg();
            pushVirtualStack();
            *_codeStrStream << "       #" << ByteCode::_name[code] << "\n";
            *_codeStrStream << "       movq    %" << arg << "," << "%" << this->getReg() << "\n";
            pushVirtualStack();
        }
            break;

        case ByteCode::_dup_x1: {
            notImplemented(code);
        }
            break;

        case ByteCode::_dup_x2: {
            notImplemented(code);
        }
            break;

        case ByteCode::_dup2: {
            notImplemented(code);
        }
            break;

        case ByteCode::_dup2_x1: {
            notImplemented(code);
        }
            break;

        case ByteCode::_dup2_x2: {
            notImplemented(code);
        }
            break;

        case ByteCode::_swap: {
            notImplemented(code);
        }
            break;

        case ByteCode::_iadd: {
            popVirtualStack();
            const char *arg1 = this->getReg();
            popVirtualStack();
            const char *arg2 = this->getReg();
            pushVirtualStack();
            *_codeStrStream << "       #" << ByteCode::_name[code] << "\n";
            *_codeStrStream << "       addq    %" << arg1 << "," << "%" << arg2 << "\n";
        }
            break;

        case ByteCode::_ladd: {
            notImplemented(code);
        }
            break;

        case ByteCode::_fadd: {
            notImplemented(code);
        }
            break;

        case ByteCode::_dadd: {
            popVirtualStack();
            const char *arg1 = this->getReg();
            popVirtualStack();
            const char *arg2 = this->getReg();
            *_codeStrStream << "      #" << ByteCode::_name[code] << "\n";
            *_codeStrStream << "      movq    %" << arg1 << ", %" << "xmm1\n";
            *_codeStrStream << "      movq    %" << arg2 << ", %" << "xmm0\n";
            *_codeStrStream << "      addsd    %" << "xmm1" << ", %" << "xmm0" << "\n";
            *_codeStrStream << "      movq    %" << "xmm0" << ", %" << getReg() << "\n";
            pushVirtualStack();
        }
            break;

        case ByteCode::_isub: {
            popVirtualStack();
            const char *arg1 = this->getReg();
            popVirtualStack();
            const char *arg2 = this->getReg();
            pushVirtualStack();
            *_codeStrStream << "       #" << ByteCode::_name[code] << "\n";
            *_codeStrStream << "       subq    %" << arg1 << "," << "%" << arg2 << "\n";
        }
            break;

        case ByteCode::_lsub: {
            notImplemented(code);
        }
            break;

        case ByteCode::_fsub: {
            notImplemented(code);
        }
            break;

        case ByteCode::_dsub: {
            popVirtualStack();
            const char *arg1 = this->getReg();
            popVirtualStack();
            const char *arg2 = this->getReg();
            *_codeStrStream << "      #" << ByteCode::_name[code] << "\n";
            *_codeStrStream << "      movq    %" << arg1 << ", %" << "xmm1\n";
            *_codeStrStream << "      movq    %" << arg2 << ", %" << "xmm0\n";
            *_codeStrStream << "      subsd    %" << "xmm1" << ", %" << "xmm0" << "\n";
            *_codeStrStream << "      movq    %" << "xmm0" << ", %" << getReg() << "\n";
            pushVirtualStack();
        }
            break;

        case ByteCode::_imul: { // TODO is this picky like idivq
            popVirtualStack();
            const char *arg1 = this->getReg();
            popVirtualStack();
            const char *arg2 = this->getReg();
            pushVirtualStack();
            *_codeStrStream << "       #" << ByteCode::_name[code] << "\n";
            *_codeStrStream << "       imulq    %" << arg1 << "," << "%" << arg2 << "\n";
        }
            break;

        case ByteCode::_lmul: {
            notImplemented(code);
        }
            break;

        case ByteCode::_fmul: {
            notImplemented(code);
        }
            break;

        case ByteCode::_dmul: {
            popVirtualStack();
            const char* arg1 = this->getReg();
            popVirtualStack();
            const char* arg2 = this->getReg();
            *_codeStrStream << "      #" << ByteCode::_name[code] << "\n";
            *_codeStrStream << "      movq    %" << arg1 << ", %" << "xmm1\n";
            *_codeStrStream << "      movq    %" << arg2 << ", %" << "xmm0\n";
            *_codeStrStream << "      mulsd    %" << "xmm1" << ", %" << "xmm0" << "\n";
            *_codeStrStream << "      movq    %" << "xmm0" << ", %" << getReg() << "\n";
            pushVirtualStack();
        }
            break;

        case ByteCode::_idiv: { // TODO
            notImplemented(code);
        }
            break;

        case ByteCode::_ldiv: {
            notImplemented(code);
        }
            break;

        case ByteCode::_fdiv: {
            notImplemented(code);
        }
            break;

        case ByteCode::_ddiv: {
            popVirtualStack();
            const char* arg1 = this->getReg();
            popVirtualStack();
            const char* arg2 = this->getReg();
            *_codeStrStream << "      #" << ByteCode::_name[code] << "\n";
            *_codeStrStream << "      movq    %" << arg1 << ", %" << "xmm1\n";
            *_codeStrStream << "      movq    %" << arg2 << ", %" << "xmm0\n";
            *_codeStrStream << "      divsd    %" << "xmm1" << ", %" << "xmm0" << "\n";
            *_codeStrStream << "      movq    %" << "xmm0" << ", %" << getReg() << "\n";
            pushVirtualStack();
        }
            break;

        case ByteCode::_irem: { // TODO (similar to idivq)
            notImplemented(code);
        }
            break;

        case ByteCode::_lrem: {
            notImplemented(code);
        }
            break;

        case ByteCode::_frem: {
            notImplemented(code);
        }
            break;

        case ByteCode::_drem: {
            notImplemented(code);
        }
            break;

        case ByteCode::_ineg: {
            popVirtualStack();
            *_codeStrStream << "       #" << ByteCode::_name[code] << "\n";
            *_codeStrStream << "       neg    %" << this->getReg() << "\n";
            pushVirtualStack();
        }
            break;

        case ByteCode::_lneg: {
            notImplemented(code);
        }
            break;

        case ByteCode::_fneg: {
            notImplemented(code);
        }
            break;

        case ByteCode::_dneg: {
            *_codeStrStream << "      #" << ByteCode::_name[code] << "\n";
            const char* arg2 = getReg();

            popVirtualStack();
            const char* arg1 = getReg();
            double neg = -1.0;
            u8* ne = (u8*) &neg;
            *_codeStrStream << "      #" << ByteCode::_name[code] << "\n";
            *_codeStrStream << "      movq    %" << arg1 << ", %" << "xmm0\n";
            *_codeStrStream << "      movq    $0x" << std::hex << *ne << ", %" << arg2<<"\n";
            *_codeStrStream << "      movq    %" << arg2 << ", %" << "xmm1\n";
            *_codeStrStream << "      mulsd    %" << "xmm1, %" << "xmm0" <<  "\n";
            *_codeStrStream << "      movq    %" << "xmm0" << ", %" << getReg() << "\n";
            pushVirtualStack();
        }
            break;

        case ByteCode::_ishl: {
            popVirtualStack();
            const char *arg1 = this->getReg();
            popVirtualStack();
            const char *arg2 = this->getReg();
            *_codeStrStream << "       #" << ByteCode::_name[code] << "\n";
            *_codeStrStream << "       sal    %" << arg1 << ",%" << arg2 << "\n";
            pushVirtualStack();
        }
            break;

        case ByteCode::_lshl: {
            notImplemented(code);
        }
            break;

        case ByteCode::_ishr: {
            popVirtualStack();
            const char *arg1 = this->getReg();
            popVirtualStack();
            const char *arg2 = this->getReg();
            *_codeStrStream << "       #" << ByteCode::_name[code] << "\n";
            *_codeStrStream << "       sar    %" << arg1 << ",%" << arg2 << "\n";
            pushVirtualStack();
        }
            break;

        case ByteCode::_lshr: {
            notImplemented(code);
        }
            break;

        case ByteCode::_iushr: {
            popVirtualStack();
            const char *arg1 = this->getReg();
            popVirtualStack();
            const char *arg2 = this->getReg();
            *_codeStrStream << "       #" << ByteCode::_name[code] << "\n";
            *_codeStrStream << "       shr    %" << arg1 << ",%" << arg2 << "\n";
            pushVirtualStack();
        }
            break;

        case ByteCode::_lushr: {
            notImplemented(code);
        }
            break;

        case ByteCode::_iand: {
            notImplemented(code);
        }
            break;

        case ByteCode::_land: {
            notImplemented(code);
        }
            break;

        case ByteCode::_ior: {
            notImplemented(code);
        }
            break;

        case ByteCode::_lor: {
            notImplemented(code);
        }
            break;

        case ByteCode::_ixor: {
            notImplemented(code);
        }
            break;

        case ByteCode::_lxor: {
            notImplemented(code);
        }
            break;

        case ByteCode::_iinc: {
            u1 *p = &codeArray[k + 1];
            u1 arg1 = ClassParser::readU1(p);
            p = &codeArray[k + 2];
            u1 arg2 = ClassParser::readU1(p);

            *_codeStrStream << "       #" << ByteCode::_name[code] << "\n";
            *_codeStrStream << "       addq    $" << arg2 << ","
                            << "-" << 8 * (_stackFrameLocalVars + arg1) << "(%rbp)\n";

//      notImplemented(code);

            /**
             * Code generation below has not been fully tested;
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


// TODO TODO are the below necessary if everything is promoted to 'long'
        case ByteCode::_i2l: {
            notImplemented(code);
        }
            break;

        case ByteCode::_i2f: {
            notImplemented(code);
        }
            break;

        case ByteCode::_i2d: {
            notImplemented(code);
        }
            break;

        case ByteCode::_l2i: {
            notImplemented(code);
        }
            break;

        case ByteCode::_l2f: {
            notImplemented(code);
        }
            break;

        case ByteCode::_l2d: {
            notImplemented(code);
        }
            break;

        case ByteCode::_f2i: {
            notImplemented(code);
        }
            break;

        case ByteCode::_f2l: {
            notImplemented(code);
        }
            break;

        case ByteCode::_f2d: {
            notImplemented(code);
        }
            break;

        case ByteCode::_d2i: {
            notImplemented(code);
        }
            break;

        case ByteCode::_d2l: {
            notImplemented(code);
        }
            break;

        case ByteCode::_d2f: {
            notImplemented(code);
        }
            break;

        case ByteCode::_i2b: {
            notImplemented(code);
        }
            break;

        case ByteCode::_i2c: {
            notImplemented(code);
        }
            break;

        case ByteCode::_i2s: {
            notImplemented(code);
        }
            break;
// TODO TODO are the below necessary if everything is promoted to 'long'

        case ByteCode::_lcmp: {
            notImplemented(code);
        }
            break;

        case ByteCode::_fcmpl: {
            notImplemented(code);
        }
            break;

        case ByteCode::_fcmpg: {
            notImplemented(code);
        }
            break;

        case ByteCode::_dcmpl: {
            notImplemented(code);
        }
            break;

        case ByteCode::_dcmpg: {
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
        case ByteCode::_if_icmple: {
            int cmp_to_zero = code < ByteCode::_if_icmpeq ? 1 : 0;
            int offset = code - (cmp_to_zero ? ByteCode::_ifeq : ByteCode::_if_icmpeq);
            std::string cmpflag = "";
            if (offset == 0) {
                cmpflag = "e";
            } else if (offset == 1) {
                cmpflag = "ne";
            } else if (offset == 2) {
                cmpflag = "l";
            } else if (offset == 3) {
                cmpflag = "ge";
            } else if (offset == 4) {
                cmpflag = "g";
            } else if (offset == 5) {
                cmpflag = "le";
            }
            //compare the twop two values on the operand stack; if succeeds, jump to the address
            u1 *p = &codeArray[k + 1];
            u2 arg = ClassParser::readU2(p) + k;
            *this->_codeStrStream << "       #"
                                  << ByteCode::_name[code] << " " << std::dec << (int) arg << "\n";

            //compare the top two numbers in the operand stack
            popVirtualStack();
            const char *reg1 = getReg();
            if (cmp_to_zero) {
                //$0 has to be the first operand for GAS/AT&T syntax
                *this->_codeStrStream << "       cmpq $0, %" << reg1 << "\n";
            } else {
                popVirtualStack();
                *this->_codeStrStream << "       cmpq %" << reg1 << ", %" << getReg() << "\n";
            }

            *this->_codeStrStream << "       j" << cmpflag << " offset_" << std::dec << arg << "\n";
        }
            break;


        case ByteCode::_if_acmpeq: {
            notImplemented(code);
        }
            break;

        case ByteCode::_if_acmpne: {
            notImplemented(code);
        }
            break;

        case ByteCode::_goto: {
            u1 *p = &codeArray[k + 1];
            u2 arg = ClassParser::readU2(p) + k;
            *this->_codeStrStream << "       #"
                                  << ByteCode::_name[code] << " " << std::dec << (int) arg << "\n";
            *this->_codeStrStream << "       jmp offset_" << std::dec << arg << "\n";
        }
            break;

        case ByteCode::_jsr: {
            notImplemented(code);
        }
            break;

        case ByteCode::_ret: {
            notImplemented(code);
        }
            break;

        case ByteCode::_tableswitch: {
            int padBytes = 3 - k % 4;
            //default bytes starts at k+1+padBytes
            u1 *p = &codeArray[k + padBytes + 1];
            int defaultOffset = (int) ClassParser::readU4(p);

            //lowbyte starts at k+1+padBytes + 4;
            p = &codeArray[k + padBytes + 5];
            int lowValue = (int) ClassParser::readU4(p);
            //highbyte starts at k+1+padBytes + 8;
            p = &codeArray[k + padBytes + 9];
            int highValue = (int) ClassParser::readU4(p);

            int numRows = highValue - lowValue + 1;
            //get top number in stack
            popVirtualStack();
            const char *reg = getReg();
            //use loop to compare it to every value in the table
            *this->_codeStrStream << "       #"
                                  << ByteCode::_name[code] << " [" << std::dec << lowValue << ", " << highValue
                                  << "]\n";

            for (int i = 0; i < numRows; i++) {
                //jump offsets starts at k+1+padBytes + 12 + 4*i;
                p = &codeArray[k + padBytes + 13 + 4 * i];
                int jumpOffset = (int) ClassParser::readU4(p);
                int jumpTo = k + jumpOffset;
                *this->_codeStrStream << "       cmpq $" << lowValue + i << ", %" << reg << "\n";
                *this->_codeStrStream << "       je offset_" << std::dec << jumpTo << "\n";

            }
            //jump to default
            *this->_codeStrStream << "       jmp offset_" << std::dec << k + defaultOffset << "\n";
        }
            break;

        case ByteCode::_lookupswitch: {
            int padBytes = 3 - k % 4;
            //npairs starts at k+1+padBytes + 4;
            u1 *p = &codeArray[k + padBytes + 5];
            int nPairs = (int) ClassParser::readU4(p);
            p = &codeArray[k + padBytes + 1];
            int defaultOffset = (int) ClassParser::readU4(p);
            //get top number in stack
            popVirtualStack();
            const char *reg = getReg();

            *this->_codeStrStream << "       #"
                                  << ByteCode::_name[code] << " (" << std::dec << nPairs << " pairs)\n";

            for (int i = 0; i < nPairs; i++) {
                p = &codeArray[k + padBytes + 9 + 8 * i];
                int caseNumber = (int) ClassParser::readU4(p);
                p = &codeArray[k + padBytes + 13 + 8 * i];
                int jumpOffset = (int) ClassParser::readU4(p);
                int jumpTo = k + jumpOffset;
                *this->_codeStrStream << "       cmpq $" << caseNumber << ", %" << reg << "\n";
                *this->_codeStrStream << "       je offset_" << std::dec << jumpTo << "\n";

            }

            //jump to default
            *this->_codeStrStream << "       jmp offset_" << std::dec << k + defaultOffset << "\n";
        }
            break;

        case ByteCode::_ireturn: {
            notImplemented(code);
        }
            break;

        case ByteCode::_lreturn: {
            notImplemented(code);
        }
            break;

        case ByteCode::_freturn: {
            notImplemented(code);
        }
            break;

        case ByteCode::_dreturn: {
            notImplemented(code);
        }
            break;

        case ByteCode::_areturn: {
            notImplemented(code);
        }
            break;

        case ByteCode::_return: {
            restoreRegsBeforeReturn();
            *_codeStrStream << "       # " << ByteCode::_name[code] << "\n";
            *_codeStrStream << "       movq    %rbp, %rsp\n";
            *_codeStrStream << "       leave\n";
            *_codeStrStream << "       ret\n";
        }
            break;

        case ByteCode::_getstatic: {
            u1 *p = &codeArray[k + 1];
            //Get Index into the constant pool
            u2 index = (u2) ClassParser::readU1(p) << 8 | (u2) ClassParser::readU1(p);
            //Get the the pointer mapped to the above index
            *_codeStrStream << "       #" << ByteCode::_name[code] << " #" << index << "\n";

            //Temporary code as a short fix to ensure we don't load a class we don't have
            CONSTANT_Fieldref_info *staticFieldInfo = (CONSTANT_Fieldref_info *) _classClass->_constantPoolInfoArray[index];
            CONSTANT_NameAndType_info *staticFieldNaT = (CONSTANT_NameAndType_info *) _classClass->_constantPoolInfoArray[staticFieldInfo->name_and_type_index];
            CONSTANT_Utf8_info *staticFieldName = (CONSTANT_Utf8_info *) _classClass->_constantPoolInfoArray[staticFieldNaT->name_index];
            if (strcmp((char *) staticFieldName->bytesArray, "out") == 0) {
                notImplemented(code);
            }
            else {
                u8 *valuePtr = _classClass->_staticVars[index];
                //Move that value into the register
                // u8 value = *valuePtr;
                *_codeStrStream << "	   movq	   $" << valuePtr << ", %rax\n"; //moved in
                *_codeStrStream << "	   movq	   (%rax)" << ", %" << getReg() << "\n"; //moved in
                pushVirtualStack();
                char formatString[5] = "%d\n";
            }
            break;
        }
            //Does not implement all of the extra type checking mentioned in JVMS since we use only 8B
        case ByteCode::_putstatic: {
            u1 *p = &codeArray[k + 1];
            //Get Index into the constant pool
            u2 index = (u2) ClassParser::readU1(p) << 8 | (u2) ClassParser::readU1(p);
            *_codeStrStream << "       #" << ByteCode::_name[code] << " #" << index << "\n";
            //Get the address that value is mapped to
            u8 *valuePtr = _classClass->_staticVars[index];
            //Move that value into the register reg = 0xDEADBEEF
            //Then set the value of the register to that dereferenced address. reg = *reg
            //popVirtualStack();
            *_codeStrStream << "	    movq	$" << valuePtr << ", %rax\n";
            *_codeStrStream << "	    movq	%" << getReg() << ", (%rax)\n";

            pushVirtualStack();
        }
            break;

        case ByteCode::_getfield: {
            notImplemented(code);
        }
            break;

        case ByteCode::_putfield: {
            notImplemented(code);
        }
            break;

        case ByteCode::_invokevirtual: {
<<<<<<< HEAD
            *this->_codeStrStream << "";
			*this->_codeStrStream << "       #"<< ByteCode::_name[code] <<"\n";
=======
            //*this->_codeStrStream << "";
            *this->_codeStrStream << "       #" << ByteCode::_name[code] << "\n";
>>>>>>> Muyuan
            popVirtualStack();
            *this->_codeStrStream << "       mov    %" << getReg() << ", %rdi\n";
            *this->_codeStrStream << "       mov    $0x" << std::hex
                                  << (unsigned long) simplePrintf << ",%rax" << "\n";
            *this->_codeStrStream << "       call   *%rax\n";
            //notImplemented(code);
        }
            break;

        case ByteCode::_invokespecial: {
            notImplemented(code);
        }
            break;

        case ByteCode::_invokestatic: {
            notImplemented(code);
        }
            break;

        case ByteCode::_invokeinterface: {
            notImplemented(code);
        }
            break;

        case ByteCode::_invokedynamic: {
            notImplemented(code);
        }
            break;

        case ByteCode::_new: {
            notImplemented(code);
        }
            break;

        case ByteCode::_newarray: {
	    //new array:
	    //the top of stack is the number of elements in the array
	    //argument shows type, using enum BasicType in ByteCode.hpp
	    u1 *p = &codeArray[k + 1];
	    u1 arg = ClassParser::readU1(p);
	    *this->_codeStrStream << "       #"
				  << ByteCode::_name[code] << " " << std::dec << (int) arg << "\n";
	    BasicType atype = (BasicType)arg;
	    *this->_codeStrStream << "       mov    $" << std::dec << atype << ", %rdi\n";
	    popVirtualStack();
	    *this->_codeStrStream << "       mov    %" << getReg() << ", %rsi\n";
            *this->_codeStrStream << "       mov    $0x" << std::hex
                                  << (unsigned long) SnapJVMRuntime::runtime_newarray << ", %rax" << "\n";
            *this->_codeStrStream << "       call   *%rax\n";
	    *this->_codeStrStream << "       movq    %rax, %" << getReg() << "\n";
            pushVirtualStack();
	    
        }
            break;

        case ByteCode::_anewarray: {
            notImplemented(code);
        }
            break;

        case ByteCode::_arraylength: {
	    *this->_codeStrStream << "       #"
				  << ByteCode::_name[code] << " \n";
	    popVirtualStack();
	    *this->_codeStrStream << "       mov    (%" << getReg() << "), %" << getReg() << "\n";
            pushVirtualStack();
        }
            break;

        case ByteCode::_athrow: {
            notImplemented(code);
        }
            break;

        case ByteCode::_checkcast: {
            notImplemented(code);
        }
            break;

        case ByteCode::_instanceof: {
            notImplemented(code);
        }
            break;

        case ByteCode::_monitorenter: {
            notImplemented(code);
        }
            break;

        case ByteCode::_monitorexit: {
            notImplemented(code);
        }
            break;

        case ByteCode::_wide: {
            notImplemented(code);
        }
            break;

        case ByteCode::_multianewarray: {
            notImplemented(code);
        }
            break;

        case ByteCode::_ifnull: {
            notImplemented(code);
        }
            break;

        case ByteCode::_ifnonnull: {
            notImplemented(code);
        }
            break;

        case ByteCode::_goto_w: {
            u1 *p = &codeArray[k + 1];
            u4 arg = ClassParser::readU4(p) + k;
            *this->_codeStrStream << "       #"
                                  << ByteCode::_name[code] << " " << std::dec << (int) arg << "\n";
            *this->_codeStrStream << "       jmp offset_" << std::dec << arg << "\n";
        }
            break;

        case ByteCode::_jsr_w: {
            notImplemented(code);
        }
            break;

        case ByteCode::_breakpoint: {
            notImplemented(code);
        }
            break;

        default:
            notImplemented(code);
            break;
    }
}

void
ByteCodeGen_X86_64::notImplemented(ByteCode::Code code) {
    *this->_codeStrStream << "       # Not implemented. "
                          << ByteCode::_name[code] << "\n";
    if (SnapJVMRuntime::isVerboseMode() || SnapJVMRuntime::isTestMode()) {
        fprintf(stderr, "ByteCodeGen_X86_64:Not implemented \"%s\"\n", ByteCode::_name[code]);
    }
}




