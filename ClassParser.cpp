/*
 * ClassParser.cpp
 *
 *  Created on: Oct 7, 2019
 *      Author: grr
 */

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

class ClassClass;

#include "ClassParser.hpp"
#include "ClassClass.hpp"
#include "ByteCodeGen_X86_64.hpp"
#include "SnapJVMRuntime.hpp"

bool ClassParser::_isLittleEndianInitialized;
bool ClassParser::_isLittleEndian;

ClassParser::ClassParser() {
	_classFileName = NULL;
	_classFileLength = 0;
	_classFileBuffer = NULL;
};

ClassClass *
ClassParser::parse(const char * classFileName)
{
	_classFileName = (char *) malloc(strlen(classFileName)+10);
	strcpy(_classFileName, classFileName);
	strcat(_classFileName,".class");

	// Get Size of file
	int fd = open(_classFileName, O_RDONLY);
	if (fd < 0) {
		perror("open");
		return NULL; // Error opening file.
	}

	// Get size of file
	_classFileLength = lseek(fd, 0L, SEEK_END);

	// Allocate memory for the class
	_classFileBuffer = (unsigned char *) malloc( _classFileLength );
	if (_classFileBuffer == NULL) {
		perror("malloc");
		close(fd);
		return NULL;
	}

	// read the class file
	lseek(fd, 0L, SEEK_SET);
	int nbytes = read(fd, _classFileBuffer, _classFileLength );
	if ( nbytes != _classFileLength) {
		// Error reading file.
		perror("read");
		close(fd);
		free(_classFileBuffer);
		return NULL;
	}

	close(fd);

	// Current position
	_u = _classFileBuffer;

	/* Format class file
	ClassFile {
	    u4             magic;
	    u2             minor_version;
	    u2             major_version;
	    u2             constant_pool_count;
	    cp_info        constant_pool[constant_pool_count-1];
	    u2             access_flags;
	    u2             this_class;
	    u2             super_class;
	    u2             interfaces_count;
	    u2             interfaces[interfaces_count];
	    u2             fields_count;
	    field_info     fields[fields_count];
	    u2             methods_count;
	    method_info    methods[methods_count];
	    u2             attributes_count;
	    attribute_info attributes[attributes_count];
	}
	*/

	// Read magic number
	u4 magic = readU4();
	if (SnapJVMRuntime::isVerboseMode()) {
		printf("magic=0x%X\n", magic);
	}

	if (magic != 0xCAFEBABE) {
		fprintf(stderr, "Not a class file\n");
		free(_classFileBuffer);
		return NULL;
	}

	// Class currently parsed
	_classClass = new ClassClass();

	// Read version numbers
	_classClass->_minor_version = readU2();
	_classClass->_major_version = readU2();

	if (SnapJVMRuntime::isVerboseMode()) {
		printf("major=%d minor=%d\n", _classClass->_major_version, _classClass->_minor_version);
	}

    parseConstantPool();

    _classClass->_access_flags = readU2();
    _classClass->_this_class = readU2();
    _classClass->_super_class = readU2();

	parseInterfaces();

	parseFields();

	parseMethods();

	return _classClass;
}

bool
ClassParser::parseConstantPool()
{
	/* Format Constant Pool
	 cp_info {
         u1 tag;
         u1 info[];
     }
	 */

	_classClass->_constant_pool_count = readU2();
	if (SnapJVMRuntime::isVerboseMode()) {
		printf("_constant_pool_count=%d\n", _classClass->_constant_pool_count);
	}

	// Create new constant pool info
	_classClass->_constantPoolInfoArray = new ConstantPoolInfoPtr[_classClass->_constant_pool_count];

	// Skip first entry in array. Entry 0 is not used.
	for (int i = 1; i < _classClass->_constant_pool_count; i++) {
		CONSTANT_POOL_TAG constantTag= (CONSTANT_POOL_TAG) readU1();
		//printf("----- i=%d tag=%d\n", i, constantTag);
		ConstantPoolInfo * cpInfo = NULL;
		int indexID = i;
		switch (constantTag) {

			/*
			CONSTANT_Class_info {
				u1 tag;
				u2 name_index;
			}
			*/
			case CONSTANT_Class: {
				CONSTANT_Class_info * c = new CONSTANT_Class_info;
				cpInfo = c;
				c->name_index = readU2();
				break;
			}

			/*
			 * CONSTANT_Fieldref_info {
				u1 tag;
			    u2 fieldIndex;
				u2 class_index;
				u2 name_and_type_index;
			}
			*/
			case CONSTANT_Fieldref: {
				CONSTANT_Fieldref_info * c = new CONSTANT_Fieldref_info;
				cpInfo = c;
				c->class_index = readU2();
				c->name_and_type_index = readU2();
				fieldCPIs.push_back(indexID);
				break;
			}

			 /*
			  * CONSTANT_Methodref_info {
				u1 tag;
				u2 class_index;
				u2 name_and_type_index;
			 }
			 */
			 case CONSTANT_Methodref: {
				CONSTANT_Methodref_info * c = new CONSTANT_Methodref_info;
				cpInfo = c;
				c->class_index = readU2();
				c->name_and_type_index = readU2();
				break;
			 }

			 /*
			  * CONSTANT_InterfaceMethodref_info {
				u1 tag;
				u2 class_index;
				u2 name_and_type_index;
			 }
			 */
			 case CONSTANT_InterfaceMethodref: {
				CONSTANT_InterfaceMethodref_info * c = new CONSTANT_InterfaceMethodref_info;
				cpInfo = c;
				c->class_index = readU2();
				c->name_and_type_index = readU2();
				break;
			 }

			 /*
			 CONSTANT_String_info {
			     u1 tag;
			     u2 string_index;
			 }
			 */

			 case CONSTANT_String: {
				CONSTANT_String_info * c = new CONSTANT_String_info;
				cpInfo = c;
				c->string_index = readU2();
				break;
			 }

			 /*
			 CONSTANT_Integer_info {
			     u1 tag;
			     u4 bytes;
			 }
			 */
			 case CONSTANT_Integer: {
				CONSTANT_Integer_info * c = new CONSTANT_Integer_info;
				cpInfo = c;
				*(u4*)&c->value = readU4();
				break;
			 }

			 /*
			 CONSTANT_Float_info {
			     u1 tag;
			     u4 bytes;
			 }
			 */
			 case CONSTANT_Float: {
				CONSTANT_Float_info * c = new CONSTANT_Float_info;
				cpInfo = c;
				*(u4 *) &c->value = readU4();
				_classClass->_constantPoolInfoArray[i] = NULL;
				break;
			 }

			 /*
			 CONSTANT_Long_info {
			     u1 tag;
			     u4 high_bytes;
			     u4 low_bytes;
			 }
			 */
			 case CONSTANT_Long: {
				CONSTANT_Long_info * c = new CONSTANT_Long_info;
				cpInfo = c;
				*(u8*)&c->value= readU8();
				// Skip entry following eigth-byte constant, see JVM book p. 98
				i++;
				_classClass->_constantPoolInfoArray[i] = NULL;
				break;
			 }

			 /*
			 CONSTANT_Double_info {
			     u1 tag;
			     u4 high_bytes;
			     u4 low_bytes;
			 }
			 */
			 case CONSTANT_Double: {
				CONSTANT_Double_info * c = new CONSTANT_Double_info;
				cpInfo = c;
				*(u8*)&c->value= readU8();
				// Skip entry following eigth-byte constant, see JVM book p. 98
				i++;
				_classClass->_constantPoolInfoArray[i] = NULL;
				break;
			 }

			 /*
			 CONSTANT_NameAndType_info {
			     u1 tag;
			     u2 name_index;
			     u2 descriptor_index;
			 }
			 */
			 case CONSTANT_NameAndType: {
				CONSTANT_NameAndType_info * c = new CONSTANT_NameAndType_info;
				cpInfo = c;
				c->name_index = readU2();
				c->descriptor_index = readU2();
				break;
			 }

			 /*
			 CONSTANT_Utf8_info {
			     u1 tag;
			     u2 length;
			     u1 bytes[length];
			 }
			 */
			 case CONSTANT_Utf8: {
				CONSTANT_Utf8_info * c = new CONSTANT_Utf8_info;
				cpInfo = c;
				c->length = readU2();
				c->bytesArray = new u1[c->length+1]; // Add space for \0
				for (int j = 0; j < c->length; j++) {
					c->bytesArray[j]=readU1();
				}
				c->bytesArray[c->length]='\0';
				break;
			 }

			 /*
			 CONSTANT_MethodHandle_info {
			     u1 tag;
			     u1 reference_kind;
			     u2 reference_index;
			 }
			 */
			 case CONSTANT_MethodHandle: {
				CONSTANT_MethodHandle_info * c = new CONSTANT_MethodHandle_info;
				cpInfo = c;
				c->reference_kind = readU1();
				c->reference_index = readU2();
				break;
			 }

			 /*
			 CONSTANT_MethodType_info {
			     u1 tag;
			     u2 descriptor_index;
			 }
			 */
			 case CONSTANT_MethodType: {
				CONSTANT_MethodType_info * c = new CONSTANT_MethodType_info;
				cpInfo = c;
				c->descriptor_index = readU2();
				break;
			 }

			 /*
			 CONSTANT_InvokeDynamic_info {
			     u1 tag;
			     u2 bootstrap_method_attr_index;
			     u2 name_and_type_index;
			 }
			 */
			 case CONSTANT_InvokeDynamic: {
				CONSTANT_InvokeDynamic_info * c = new CONSTANT_InvokeDynamic_info;
				cpInfo = c;
				c->bootstrap_method_attr_index = readU2();
				c->name_and_type_index = readU2();
				break;
			 }

			 default: {
				 fprintf(stderr, "ClassParser: Tag not found %d:%d\n", i, constantTag);
				 exit(1);
			 }
		}

		cpInfo->index_id = indexID;
		cpInfo->tag = constantTag;
		_classClass->_constantPoolInfoArray[indexID] = cpInfo;
		//cpInfo->print(this);
	}

	return true;
}

bool
ClassParser::parseInterfaces()
{
	// Create _interfacesArray
	_classClass->_interfaces_count = readU2();
	_classClass->_interfacesArray = new u2[_classClass->_interfaces_count];

	// Skip first entry in array. Entry 0 is not used.
	for (int i = 0; i < _classClass->_interfaces_count; i++) {
		_classClass->_interfacesArray[i] = readU2();
	}
	return true;
}

bool
ClassParser::parseFields()
{
	/*
		field_info {
			u2             access_flags;
			u2             name_index;
			u2             descriptor_index;
			u2             attributes_count;
			attribute_info attributes[attributes_count];
		}

		attribute_info {
			u2 attribute_name_index;
			u4 attribute_length;
			u1 info[attribute_length];
		}
	*/

    u2 numLocalFields = 0;
    u2 numStaticFields = 0;
	_classClass->_fields_count = readU2();
	_classClass->_fieldsArray = new FieldInfoPtr[_classClass->_fields_count];
	for (int i = 0; i < _classClass->_fields_count; i++) {
		FieldInfo * fieldInfo = new FieldInfo;
		_classClass->_fieldsArray[i] = fieldInfo;
		fieldInfo->access_flags = readU2();
		fieldInfo->name_index = readU2();
		fieldInfo->descriptor_index = readU2();
		fieldInfo->attributes_count = readU2();
		fieldInfo->attributesArray = new AttributeInfoPtr[fieldInfo->attributes_count];
		for (int j = 0; j < fieldInfo->attributes_count; j++) {
			AttributeInfo * attributeInfo = new AttributeInfo;
			fieldInfo->attributesArray[j]=attributeInfo;
			attributeInfo->attribute_name_index = readU2();
			attributeInfo->attribute_length = readU4();
			attributeInfo->infoArray = new u1[attributeInfo->attribute_length];
			for (int k = 0; k < attributeInfo->attribute_length; k++) {
				attributeInfo->infoArray[k] = readU1();
			}
		}
		//Add Field to the appropriate map
		if ((fieldInfo->access_flags & 0x0008) == 0x0008){
			_classClass->_staticVars[fieldCPIs.front()] = numStaticFields++;
		} else _classClass->_instanceVars[fieldCPIs.front()] = numLocalFields++;
		fieldCPIs.pop_front();
	}
	_classClass->_staticVariables = (u8 *) malloc(sizeof(u8) * numStaticFields);
	return true;
}

bool
ClassParser::parseMethods()
{
	/*
	method_info {
	    u2             access_flags;
	    u2             name_index;
	    u2             descriptor_index;
	    u2             attributes_count;
	    attribute_info attributes[attributes_count];
	}
	attribute_info {
		u2 attribute_name_index;
		u4 attribute_length;
		u1 info[attribute_length];
	}
	*/
	_classClass->_methods_count = readU2();
	_classClass->_methodInfoArray = new MethodInfoPtr[_classClass->_methods_count];
	for ( int i = 0; i < _classClass->_methods_count; i++) {
		MethodInfo * methodInfo = new MethodInfo;
		_classClass->_methodInfoArray[i] = methodInfo;
		methodInfo->access_flags = readU2();
		methodInfo->name_index = readU2();
		methodInfo->descriptor_index = readU2();
		methodInfo->attributes_count = readU2();
		methodInfo->attributesArray = new AttributeInfoPtr[methodInfo->attributes_count];
		for (int j = 0; j < methodInfo->attributes_count; j++) {
			AttributeInfo * attributeInfo = new AttributeInfo;
			methodInfo->attributesArray[j]=attributeInfo;
			attributeInfo->attribute_name_index = readU2();
			attributeInfo->attribute_length = readU4();
			attributeInfo->infoArray = new u1[attributeInfo->attribute_length];
			for (int k = 0; k < attributeInfo->attribute_length; k++) {
				attributeInfo->infoArray[k] = readU1();
			}
		}
	}
}




