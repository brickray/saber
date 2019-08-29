#ifndef H_SABER_H
#define H_SABER_H

#include "common.h"
#include "error.h"
#include "value.h"

SABER_NAMESPACE_BEGIN

inline void checkParamsNum(string func, int numParams, int expect = 1){
	if (numParams != expect){
		Error::GetInstance()->ProcessError("%s函数只接收%d个参数\n", func.c_str(), expect);
	}

	return;
}

inline void checkBoolean(string func, Value& v, int idx = 1){
	if (!v.IsBoolean()){
		Error::GetInstance()->ProcessError("%s函数第%d个参数类型需为boolean\n", func.c_str(), idx);
	}
}

inline void checkNumber(string func, Value& v, int idx = 1){
	if (!v.IsNumber()){
		Error::GetInstance()->ProcessError("%s函数第%d个参数类型需为number\n", func.c_str(), idx);
	}
}

inline void checkInteger(string func, Value& v, int idx = 1){
	if (!v.IsInteger()){
		Error::GetInstance()->ProcessError("%s函数第%d个参数类型需为integer\n", func.c_str(), idx);
	}
}

inline void checkFloat(string func, Value& v, int idx = 1){
	if (!v.IsFloat()){
		Error::GetInstance()->ProcessError("%s函数第%d个参数类型需为float\n", func.c_str(), idx);
	}
}

inline void checkString(string func, Value& v, int idx = 1){
	if (!v.IsString()){
		Error::GetInstance()->ProcessError("%s函数第%d个参数类型需为string\n", func.c_str(), idx);
	}
}

inline void checkFunction(string func, Value& v, int idx = 1){
	if (!v.IsFunction()){
		Error::GetInstance()->ProcessError("%s函数第%d个参数类型需为function\n", func.c_str(), idx);
	}
}

inline void checkNativeFunction(string func, Value& v, int idx = 1){
	if (!v.IsNativeFunction()){
		Error::GetInstance()->ProcessError("%s函数第%d个参数类型需为nativefunction\n", func.c_str(), idx);
	}
}

inline void checkLightUData(string func, Value& v, int idx = 1){
	if (!v.IsLightUData()){
		Error::GetInstance()->ProcessError("%s函数第%d个参数类型需为lightudata\n", func.c_str(), idx);
	}
}

inline void checkTable(string func, Value& v, int idx = 1){
	if (!v.IsTable()){
		Error::GetInstance()->ProcessError("%s函数第%d个参数类型需为table\n", func.c_str(), idx);
	}
}

SABER_NAMESPACE_END

#endif