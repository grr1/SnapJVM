/*
 * ClassClass.h
 *
 *  Created on: Nov 1, 2019
 *      Author: grr
 */

#ifndef CLASSCLASS_H_
#define CLASSCLASS_H_

class Method;

#include "ByteCode.hpp"
#include "ClassParser.hpp"

class ConstantPoolInfo;
typedef ConstantPoolInfo * ConstantPoolInfoPtr;

class FieldInfo;
typedef FieldInfo * FieldInfoPtr;

class MethodInfo;
typedef MethodInfo * MethodInfoPtr;

class ClassClass {
	friend class ClassParser;

public:
	///////////////////////////////////////////////////////////////
	// Data from the ClassParser
	///////////////////////////////////////////////////////////////
	char * _classFileName;

	// Header
	u4 _magic;
	u2 _minor_version;
	u2 _major_version;

	// Constant Pool
	u2 _constant_pool_count;
	ConstantPoolInfoPtr *_constantPoolInfoArray;

	u2 _access_flags;
	u2 _this_class;
	u2 _super_class;

	u2 _interfaces_count;
	u2 * _interfacesArray;

    u2 _fields_count;
    FieldInfoPtr * _fieldsArray;

    u2 _methods_count;
    MethodInfoPtr *_methodInfoArray;

	// Array with all methods in this class
	int nMethods;
	Method * _methodsArray;

	//Array with all static variables
	u8 * _staticVariables;

	//Map for all static variables
	map <String, u2> _staticVars;

	//Map for all instance variables
	map <String, u2> _instanceVars;

	//Map for all class methods
    map <String, Method> _classMethods;

public:
	ClassClass();
	virtual ~ClassClass();

	void print();
	void printStringDecorated(int length, u1 * bytesArray);
	void printStringDecorated(u2 nameIndex);
	void printClassName(u2 classIndex);
	void printNameAndType(u2 nameAndTypeIndex);
	void printExtendedType(u2 descriptorIndex);
	void printFlags(u2 flags);
	int runMain(int argc, char **argv);
	Method * lookupMethod(const char * methodName);
	u8 getField(string);
	void addField(string, u8);
};


enum CONSTANT_POOL_TAG {
	CONSTANT_Class =7,
	CONSTANT_Fieldref=9,
	CONSTANT_Methodref =10,
	CONSTANT_InterfaceMethodref=11,
	CONSTANT_String=8,
	CONSTANT_Integer=3,
	CONSTANT_Float=4,
	CONSTANT_Long=5,
	CONSTANT_Double=6,
	CONSTANT_NameAndType=12,
	CONSTANT_Utf8=1,
	CONSTANT_MethodHandle=15,
	CONSTANT_MethodType=16,
	CONSTANT_InvokeDynamic=18
};

class ConstantPoolInfo {
public:
	u2 index_id;
	CONSTANT_POOL_TAG tag;
	virtual void print(ClassClass * classClass) {
		printf("#%-3d = tag=%d\n", index_id, tag);
	}
	virtual void printShort(ClassClass * classClass) {
			printf(" ");
	}
	virtual u1 * toData(ClassClass *classClass) {
		return NULL;
	}
};

// Example javap format:
// #1 = Methodref          #6.#15         // java/lang/Object."<init>":()V

class CONSTANT_Class_info : public ConstantPoolInfo {
public:
    u2 name_index;
    void print(ClassClass * classClass) {
		printf("#%-3d= %-20s #%d\t\t// ", this->index_id, "Class", name_index);
		classClass->printStringDecorated(name_index);
		printf("\n");
	}
    void printShort(ClassClass * classClass) {
    	classClass->printStringDecorated(name_index);
	}

};

class CONSTANT_Fieldref_info : public ConstantPoolInfo {
public:
	u2 class_index;
    u2 name_and_type_index;
    void print(ClassClass * classClass) {
    	printf("#%-3d= %-20s #%d.#%d\t// ", this->index_id, "Fieldref", class_index, name_and_type_index);
    	classClass->printClassName(class_index);
		printf(".");
		classClass->printNameAndType(name_and_type_index);
		printf("\n");
    }
    void printShort(ClassClass * classClass) {
    	classClass->printClassName(class_index);
		printf(".");
		classClass->printNameAndType(name_and_type_index);
    }
};

class CONSTANT_Methodref_info : public ConstantPoolInfo {
public:
	u2 class_index;
    u2 name_and_type_index;
    void print(ClassClass * classClass) {
    	printf("#%-3d= %-20s #%d.#%d\t// ", this->index_id, "Methodref", class_index, name_and_type_index);
    	classClass->printClassName(class_index);
		printf(".");
		classClass->printNameAndType(name_and_type_index);
		printf("\n");
	}
    void printShort(ClassClass * classClass) {
    	classClass->printClassName(class_index);
		printf(".");
		classClass->printNameAndType(name_and_type_index);
    }
};

class CONSTANT_InterfaceMethodref_info : public ConstantPoolInfo {
public:
	u2 class_index;
    u2 name_and_type_index;
    void print(ClassClass * classClass) {
        printf("#%-3d= %-20s #%d.#%d\t// ", this->index_id, "InterfaceMethodref", class_index, name_and_type_index);
        classClass->printClassName(class_index);
        printf(".");
        classClass->printNameAndType(name_and_type_index);
        printf("\n");
    }
    void printShort(ClassClass * classClass) {
    	classClass->printClassName(class_index);
        printf(".");
        classClass->printNameAndType(name_and_type_index);
    }
};

class CONSTANT_Integer_info : public ConstantPoolInfo {
public:
    int value;
    void print(ClassClass * classClass) {
    	printf("#%-3d= %-20s %d\n", this->index_id, "Integer", value);
    }
    void printShort(ClassParser * classParser) {
    	printf("#%-3d= %-20s %d", this->index_id, "Integer", value);
    }
    u1 * toData(ClassClass *classClass) {
    	return (u1*) &value;
    }
};

class CONSTANT_Float_info : public ConstantPoolInfo {
public:
	float value;
	void print(ClassClass * classClass) {
		printf("#%-3d= %-20s %.10ff\n", this->index_id, "Float", value);
	}
    void printShort(ClassClass * classClass) {
    	printf("#%-3d= %-20s %.10ff", this->index_id, "Float", value);
    }
    u1 * toData(ClassClass *classClass) {
    	return (u1*) &value;
    }
 };

class CONSTANT_Long_info : public ConstantPoolInfo {
public:
    long int value;
    void print(ClassClass * classClass) {
    	printf("#%-3d = %-20s %ldL\n", this->index_id, "Long", value);
    }
    void printShort(ClassClass * classClass) {
    	printf("#%-3d = %-20s %ldL", this->index_id, "Long", value);
    }
    u1 * toData(ClassClass *classClass) {
    	return (u1*) &value;
    }
};

class CONSTANT_Double_info : public ConstantPoolInfo {
public:
    double value;
    void print(ClassClass * classClass) {
    	printf("#%-3d= %-20s %.10lfd\n", this->index_id, "Double", value);
    }
    void printShort(ClassClass * classClass) {
    	printf("#%-3d= %-20s %.10lfd", this->index_id, "Double", value);
    }
    u1 * toData(ClassClass *classClass) {
       return (u1*) &value;
    }
};

class CONSTANT_NameAndType_info : public ConstantPoolInfo {
public:
    u2 name_index;
    u2 descriptor_index;
    void print(ClassClass * classClass) {
		printf("#%-3d= %-20s #%d:#%d\t\t// ", this->index_id, "NameAndType", name_index, descriptor_index);
		printf("\"");
		classClass->printStringDecorated(name_index);
		printf("\":");
		classClass->printStringDecorated(descriptor_index);
		printf("\n");
	}
    void printShort(ClassClass * classClass) {
    	printf("\"");
    	classClass->printStringDecorated(name_index);
		printf("\":");
		classClass->printStringDecorated(descriptor_index);
    }
};

class CONSTANT_Utf8_info : public ConstantPoolInfo {
public:
    u2 length;
    u1 * bytesArray; // Array of bytes
    void print(ClassClass * classClass) {
    	printf("#%-3d= %-20s ", this->index_id, "Utf8");
    	classClass->printStringDecorated(length, bytesArray);
    	printf("\n");
    }
    void printShort(ClassClass * classClass) {
    	classClass->printStringDecorated(length, bytesArray);
    }
    u1 * toData(ClassClass *classClass) {
    	return bytesArray;
    }
};

class CONSTANT_String_info : public ConstantPoolInfo {
public:
    u2 string_index;
    void print(ClassClass * classClass) {
    	printf("#%-3d= %-20s #%d\t\t // ", this->index_id, "String", string_index);
    	classClass->printStringDecorated(string_index);
    	printf("\n");
    }
    void printShort(ClassClass * classClass) {
    	classClass->printStringDecorated(string_index);
    }

    u1 * toData(ClassClass * classClass) {
    	ConstantPoolInfo *info = classClass->_constantPoolInfoArray[string_index];
		CONSTANT_Utf8_info * uinfo = dynamic_cast<CONSTANT_Utf8_info *>(info);
		if (uinfo == NULL) {
			printf("(no UTF index #%d)", string_index);
			return NULL;
		}
    	return uinfo->bytesArray;
    }
};

class CONSTANT_MethodHandle_info : public ConstantPoolInfo {
public:
    u1 reference_kind;
    u2 reference_index;
};

class CONSTANT_MethodType_info : public ConstantPoolInfo {
public:
    u2 descriptor_index;
};

class CONSTANT_InvokeDynamic_info : public ConstantPoolInfo {
public:
    u2 bootstrap_method_attr_index;
    u2 name_and_type_index;
};

///////////////////

class AttributeInfo;
typedef AttributeInfo * AttributeInfoPtr;

class FieldInfo {
public:
	u2 access_flags;
	u2 name_index;
	u2 descriptor_index;
	u2 attributes_count;
	AttributeInfoPtr * attributesArray;
};

enum MethodAttributeType {
	MethodConstantValue,
	MethodCode,
	MethodStackMapTable,
	MethodExceptions,
	MethodInnerClasses,
	MethodEnclosingMethod,
	MethodSynthetic,
	MethodSignature,
	MethodSourceFile,
	MethodSourceDebugExtension,
	MethodLineNumberTable,
	MethodLocalVariableTable,
	MethodLocalVariableTypeTable,
	MethodDeprecated,
	MethodRuntimeVisibleAnnotations,
	MethodRuntimeInvisibleAnnotations,
	MethodRuntimeVisibleParameterAnnotations,
	MethodRuntimeInvisibleParameterAnnotations,
	MethodAnnotationDefault,
	MethodBootstrapMethods
};

class AttributeInfo {
public:
	u2 attribute_name_index;
	u4 attribute_length;
	u1 *infoArray;
};

class MethodInfo {
public:
	u2 access_flags;
	u2 name_index;
	u2 descriptor_index;
	u2 attributes_count;
	AttributeInfoPtr *attributesArray;
};

class ExceptionTable {
public:
	u2 start_pc;
	u2 end_pc;
	u2 handler_pc;
	u2 catch_type;
};

class CodeAttribute {
public:
    u2 attribute_name_index;
    u4 attribute_length;
    u2 max_stack;
    u2 max_locals;
    u4 code_length;
    u1 * codeArray;
    u2 exception_table_length;
    ExceptionTable * exceptionTableArray;
    u2 attributes_count;
    AttributeInfoPtr * attributesArray;
};

#endif /* CLASSCLASS_H_ */
