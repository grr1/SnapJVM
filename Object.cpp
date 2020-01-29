/*
 * Object.cpp
 *
 *  Created on: Nov 1, 2019
 *      Author: grr
 */

#include "Object.h"

Object::Object() {
	// TODO Auto-generated constructor stub

}

Object::Object(ClassClass * ){
    _class = myClass;
    int numfields = 0;
    for (int i = 0; i < _class->fields_count; i++){
        FieldInfo * currentField = _class->_fieldsArray[i];
        if ((currentField->access_flags & 0x0008) != 0x0008){ //If not static
            numfields++;
        }
    }
    _localVariables = malloc(u8 * numfields);
}

u8 Object::getField(string fieldName){
    u2 varIndex = _class->_instanceVars[fieldName];
    return _localVariables[varIndex];
}

Object::~Object() {
	// TODO Auto-generated destructor stub
}

