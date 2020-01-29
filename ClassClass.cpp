/*
 * ClassClass.cpp
 *
 *  Created on: Nov 1, 2019
 *      Author: grr
 */

#include "ClassClass.hpp"
#include "Method.hpp"

ClassClass::ClassClass() {


}

ClassClass::~ClassClass() {

}

int
ClassClass::runMain(int argc, char **argv) {\
	Method * method = lookupMethod("main");
	if (method ==NULL) {
		fprintf(stderr, "Class does not have \"main\" method");
		return -1;
	}

	// Call main
	void * (*fptr)(int, char**) = (void * (*)(int, char **))method->_code;
	(*fptr)(argc, argv);
}

Method *
ClassClass::lookupMethod(const char * methodName)
{
	for(int i =0; i<this->_methods_count; i++) {
		Method * method = &this->_methodsArray[i];
		if (!strcmp(method->_methodName, methodName)) {
			return method;
		}
	}
	return NULL;
}

///////////////////////////////////////////////////
// ClassClass Printers
///////////////////////////////////////////////////

void
ClassClass::printStringDecorated(int length, u1 * bytesArray){
	// Print with expanded characters
	for (int i = 0; i < length; i++) {
		u1 ch = bytesArray[i];
		if ( ch=='\n') printf("\\n");
		else if (ch <32 || ch >127) printf("\\0%o", ch);
		else printf("%c", ch);
	}
}

void
ClassClass::printStringDecorated(u2 nameIndex)
{
	ConstantPoolInfo *info = _constantPoolInfoArray[nameIndex];

	CONSTANT_Utf8_info * uinfo = dynamic_cast<CONSTANT_Utf8_info *>(info);
	if (uinfo == NULL) {
		printf("(no UTF index #%d)", nameIndex);
		return;
	}
	printStringDecorated(uinfo->length, uinfo->bytesArray);
}

void
ClassClass::printClassName(u2 classIndex)
{
	ConstantPoolInfo *info = _constantPoolInfoArray[classIndex];

	CONSTANT_Class_info * cinfo = dynamic_cast<CONSTANT_Class_info *>(info);
	if (cinfo == NULL) {
		printf("(no Class index #%d)", classIndex);
		return;
	}
	printStringDecorated(cinfo->name_index);
}

void
ClassClass::printNameAndType(u2 nameAndTypeIndex)
{
	ConstantPoolInfo *info = _constantPoolInfoArray[nameAndTypeIndex];

	CONSTANT_NameAndType_info * cinfo = dynamic_cast<CONSTANT_NameAndType_info *>(info);
	if (cinfo == NULL) {
		printf("(no NameAndType index #%d)", nameAndTypeIndex);
		return;
	}
	printf("\"");
	printStringDecorated(cinfo->name_index);
	printf("\":");
	printStringDecorated(cinfo->descriptor_index);
}

void
ClassClass::printExtendedType(u2 descriptorIndex) {
	/*
	B 	byte 	signed byte
	C 	char 	Unicode character code point in the Basic Multilingual Plane, encoded with UTF-16
	D 	double 	double-precision floating-point value
	F 	float 	single-precision floating-point value
	I 	int 	integer
	J 	long 	long integer
	L ClassName ; 	reference 	an instance of class ClassName
	S 	short 	signed short
	Z 	boolean 	true or false
	[ 	reference 	one array dimension

	ConstantPoolInfo *info = _constantPoolInfoArray[descriptorIndex];
	*/

	const char * baseTypeCharacter[][2] = {
			{"B","byte"},
			{"C","char"},
			{"D","double"},
			{"F","float"},
			{"I","int"},
			{"J","long"},
			{"L","ClassName"},
			{"S","short"},
			{"[","reference"}
	};

	ConstantPoolInfo *info = _constantPoolInfoArray[descriptorIndex];
	CONSTANT_Utf8_info * uinfo = dynamic_cast<CONSTANT_Utf8_info *>(info);
	if (uinfo == NULL) {
		printf("(no UTF index #%d)", descriptorIndex);
		return;
	}

	if (uinfo->bytesArray[0]=='L') {
		// class name. Transform '/' to '.'
		for (int j = 1; j < uinfo->length; j++) {
			if (uinfo->bytesArray[j]=='/') {
				printf(".");
			}
			else {
				printf("%c",uinfo->bytesArray[j]);
			}
		}
		return;
	}

	// Primitive type
	int nBaseType = sizeof(baseTypeCharacter)/(2*sizeof(char*));
	int i;
	for (i = 0; i < nBaseType; i++) {
		if (!strcmp((const char*)uinfo->bytesArray,baseTypeCharacter[i][0])) {
			printf("%s",baseTypeCharacter[i][1]);
			break;
		}
	}
}

void
ClassClass::printFlags(u2 flags) {

	if (flags & ByteCode::Flags::_bc_can_trap) printf("can trap,");
	if (flags & ByteCode::Flags::_bc_can_rewrite) printf("can rewrite,");
	if (flags & ByteCode::Flags::_fmt_has_c) printf("has_c,");
	if (flags & ByteCode::Flags::_fmt_has_j) printf("has_j,");
	if (flags & ByteCode::Flags::_fmt_has_k) printf("fmt_has_k,");
	if (flags & ByteCode::Flags::_fmt_has_i) printf("mt_has_i,");
	if (flags & ByteCode::Flags::_fmt_has_o) printf("fmt_has_o,");
	if (flags & ByteCode::Flags::_fmt_has_nbo) printf("has_nbo,");
	if (flags & ByteCode::Flags::_fmt_has_u2) printf("has_u2,");
	if (flags & ByteCode::Flags::_fmt_has_u4) printf("has_u4,");
	if (flags & ByteCode::Flags::_fmt_not_variable) printf("not_variable,");
	if (flags & ByteCode::Flags::_fmt_not_simple) printf("not_simple,");

	if (flags == ByteCode::Flags::_fmt_b) printf("_fmt_b,");
	if (flags == ByteCode::Flags::_fmt_bc) printf("_fmt_bc,");
	if (flags == ByteCode::Flags::_fmt_bi) printf("_fmt_bi,");
	if (flags == ByteCode::Flags::_fmt_bkk) printf("_fmt_bkk,");
	if (flags == ByteCode::Flags::_fmt_bJJ) printf("_fmt_bJJ,");
	if (flags == ByteCode::Flags::_fmt_bo2) printf("_fmt_bo2,");
	if (flags == ByteCode::Flags::_fmt_bo4) printf("_fmt_bo4,");

	//if (flags & ByteCode::Flags::_fmt_has_j) {
		printf(" flags=0x%x\n", flags);
	//}

}

std::string
ClassClass::getFieldName(u2 constantPoolIndex){
	ConstantPoolInfo * cpi1 = _constantPoolInfoArray[constantPoolIndex];
	CONSTANT_NameAndType_info * natinfo = dynamic_cast<CONSTANT_NameAndType_info*>(cpi1);
	ConstantPoolInfo * cpi2 = _constantPoolInfoArray[natinfo->name_index];
	CONSTANT_Utf8_info * nameString = dynamic_cast<CONSTANT_Utf8_info*>(cpi2);
	return std::string((char *) nameString->toData(this));
}

u8
ClassClass::getField(std::string fieldName){
	return _staticVariables[_staticVars[fieldName]];
}

void
ClassClass::addField(std::string fieldName, u8 value){
	if (_staticVariables == nullptr){
		_staticVariables = (u8 *) malloc(_staticVars.size() * value);
	}
	_staticVariables[_staticVars[fieldName]] = value;
}

void
ClassClass::print()
{
	printf("Classfile %s\n", _classFileName);

	printf("class "); printClassName(this->_this_class); printf("\n");
	printf("  minor version: %d\n", this->_minor_version);
	printf("  major version: %d\n", this->_major_version);
	printf("  flags: (0x%04X)\n", this->_access_flags);
	printf("  this_class: #%d\t\t// ", this->_this_class);
	printClassName(this->_this_class); printf("\n");
	printf("  super_class: #%d\t\t// ", this->_super_class);
	printClassName(this->_super_class); printf("\n");
	printf("  interfaces: %d, fields: %d, methods: %d, attributes: %d\n",
			_interfaces_count, 0,0,0);

	printf("Constant pool:\n");
	for (int i = 1; i < _constant_pool_count; i++) {
		ConstantPoolInfoPtr info = _constantPoolInfoArray[i];
		if (info != NULL) { // skip entries with no info
			printf("  ");info->print(this);
		}
	}

	if (_interfaces_count > 0) {
		printf("Interfaces:\n");
		for (int i = 0; i < _interfaces_count; i++) {
			u2 classIndex = _interfacesArray[i];
			printf("  "); printClassName(classIndex);
		}
		printf("\n");
	}

	printf("{\n");

	// Print fields
	for (int i = 0; i < _fields_count; i++) {
		FieldInfo * fieldInfo = _fieldsArray[i];
		// Print type
		printf("  "); printExtendedType(fieldInfo->descriptor_index);
		printf(" "); printStringDecorated(fieldInfo->name_index);
		printf(";\n");
		printf("    descriptor: ");
		printStringDecorated(fieldInfo->descriptor_index);
		printf("\n");
		printf("    flags: (0x%04X)\n",fieldInfo->access_flags);
		printf("    attributes_count: %d\n",fieldInfo->attributes_count);
		printf("\n");
	}

	// Print methods
	for (int i = 0; i < _methods_count; i++) {
		MethodInfo * methodInfo = _methodInfoArray[i];
		// Print type
		printf("  "); printExtendedType(methodInfo->descriptor_index);
		printf(" "); printStringDecorated(methodInfo->name_index);
		printf(";\n");
		printf("    descriptor: ");
		printStringDecorated(methodInfo->descriptor_index);
		printf("\n");
		printf("    flags: (0x%04X)\n",methodInfo->access_flags);
		printf("    attributes_count: %d\n",methodInfo->attributes_count);
		for (int j = 0; j < methodInfo->attributes_count; j++) {
			AttributeInfo * attrInfo = methodInfo->attributesArray[j];
			printf("    %d:",j); printStringDecorated(attrInfo->attribute_name_index);
			printf("\n");
			printf("    attributes_count=%d\n", attrInfo->attribute_length);
			printf("\n");
			ConstantPoolInfo *info = _constantPoolInfoArray[attrInfo->attribute_name_index];
			CONSTANT_Utf8_info * uinfo = dynamic_cast<CONSTANT_Utf8_info *>(info);
			const char * sname = "";
			if (uinfo != NULL) {
				sname = (const char *)uinfo->bytesArray;
			}
			if (!strcmp(sname,"Code")) {
				//CodeAttribute * codeAttribute = ()
				unsigned char *uptr = attrInfo->infoArray;
				u2 max_stack = ClassParser::readU2(uptr);
				u2 max_locals = ClassParser::readU2(uptr);
				u4 code_length = ClassParser::readU4(uptr);
				u1 * codeArray = uptr;
				int k = 0;
				while (k<code_length) {
					ByteCode::Code c =  (ByteCode::Code) codeArray[k];
					printf("         %d: %s ", k, ByteCode::_name[c]);
					u2 flags = ByteCode::_flags[c];
					if (flags & ByteCode::Flags::_fmt_has_j &&
						flags & ByteCode::Flags::_fmt_has_u2	) {
						u1 * p = uptr+1;
						u2 arg = ClassParser::readU2(p);
						printf(" #%d",arg);
						ConstantPoolInfoPtr info = _constantPoolInfoArray[arg];
						if (info != NULL) { // skip entries with no info
							printf("\t// ");info->printShort(this);
						}
					}
					else if (flags & ByteCode::Flags::_fmt_has_k &&
						flags & ByteCode::Flags::_fmt_has_u2	) {
						u1 * p = uptr+1;
						u2 arg = ClassParser::readU2(p);
						printf(" #%d",arg);
						ConstantPoolInfoPtr info = _constantPoolInfoArray[arg];
						if (info != NULL) { // skip entries with no info
							printf("\t// ");info->printShort(this);
						}
					}
					else if (flags & ByteCode::Flags::_fmt_has_k &&
							!(flags & ByteCode::Flags::_fmt_has_u2)) {
						u1 * p = uptr+1;
						u1 arg = ClassParser::readU1(p);
						printf(" #%d",arg);
						ConstantPoolInfoPtr info = _constantPoolInfoArray[arg];
						if (info != NULL) { // skip entries with no info
							printf("\t// ");info->printShort(this);
						}
					}
					else if ((flags & ByteCode::Flags::_fmt_bc) == ByteCode::Flags::_fmt_bc) {
							u1 * p = uptr+1;
							u1 arg = ClassParser::readU1(p);
							printf(" %d",arg);
					}
					printf("\n");

					printFlags(flags);
					//printf("         %d: %d\n",k, attrInfo->infoArray[k]);
					//k++;
					k+=ByteCode::_lengths[c];
					uptr += ByteCode::_lengths[c];;
				}
				printf("\n");
			}
		}
		printf("\n");
	}

	printf("}\n");
}


