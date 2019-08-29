#ifndef H_SABER_H
#define H_SABER_H

#include "common.h"
#include "error.h"
#include "value.h"

SABER_NAMESPACE_BEGIN

inline void checkParamsNum(string func, int numParams, int expect = 1){
	if (numParams != expect){
		Error::GetInstance()->ProcessError("%s����ֻ����%d������\n", func.c_str(), expect);
	}

	return;
}

inline void checkBoolean(string func, Value& v, int idx = 1){
	if (!v.IsBoolean()){
		Error::GetInstance()->ProcessError("%s������%d������������Ϊboolean\n", func.c_str(), idx);
	}
}

inline void checkNumber(string func, Value& v, int idx = 1){
	if (!v.IsNumber()){
		Error::GetInstance()->ProcessError("%s������%d������������Ϊnumber\n", func.c_str(), idx);
	}
}

inline void checkInteger(string func, Value& v, int idx = 1){
	if (!v.IsInteger()){
		Error::GetInstance()->ProcessError("%s������%d������������Ϊinteger\n", func.c_str(), idx);
	}
}

inline void checkFloat(string func, Value& v, int idx = 1){
	if (!v.IsFloat()){
		Error::GetInstance()->ProcessError("%s������%d������������Ϊfloat\n", func.c_str(), idx);
	}
}

inline void checkString(string func, Value& v, int idx = 1){
	if (!v.IsString()){
		Error::GetInstance()->ProcessError("%s������%d������������Ϊstring\n", func.c_str(), idx);
	}
}

inline void checkFunction(string func, Value& v, int idx = 1){
	if (!v.IsFunction()){
		Error::GetInstance()->ProcessError("%s������%d������������Ϊfunction\n", func.c_str(), idx);
	}
}

inline void checkNativeFunction(string func, Value& v, int idx = 1){
	if (!v.IsNativeFunction()){
		Error::GetInstance()->ProcessError("%s������%d������������Ϊnativefunction\n", func.c_str(), idx);
	}
}

inline void checkLightUData(string func, Value& v, int idx = 1){
	if (!v.IsLightUData()){
		Error::GetInstance()->ProcessError("%s������%d������������Ϊlightudata\n", func.c_str(), idx);
	}
}

inline void checkTable(string func, Value& v, int idx = 1){
	if (!v.IsTable()){
		Error::GetInstance()->ProcessError("%s������%d������������Ϊtable\n", func.c_str(), idx);
	}
}

SABER_NAMESPACE_END

#endif