#ifndef H_VALUE_H
#define H_VALUE_H

#include "common.h"
#include "error.h"

SABER_NAMESPACE_BEGIN

enum EValueType{
	EBOOLEAN    = 1 << 0,
	EINTEGER    = 1 << 1,
	EFLOAT      = 1 << 2,
	ESTRING     = 1 << 3,
	EFUNC       = 1 << 4,
	ENATIVEFUNC = 1 << 5,
	ELIGHTUDATA = 1 << 6,
	ETABLE      = 1 << 7,
	ECOROUTINE  = 1 << 8,
	ENULL       = 1 << 9,
	ENUMBER     = EINTEGER | EFLOAT,
};

class Environment;
class Astree;
class SVM;
typedef int(*SFunc)(SVM* vm, int numParams);

enum class ECoroutineStatus{
	ESTART     = 0,
	ESUSPENDED = 1,
	ERUNING    = 2,
	EDEAD      = 3,
};

class Value;
class Closure;
typedef shared_ptr<Closure> Clptr;
struct Coroutine{
	ECoroutineStatus status;
	Clptr cl;
	int ip;
};

class Closure;
struct SValue{
	union{
		bool       bValue;
		int        iValue;
		float      fValue;
		SFunc      sfunc;
		Coroutine* co;
	};
	Clptr  cl;
	string sValue;
};

struct Table{
	hash_map<string, Value> kv;
};

typedef hash_map<string, Value>::iterator TableIteration;

class Value{
private:
	SValue value;
	EValueType type;

public:
	Value(){ type = EValueType::ENULL; }
	Value(EValueType t, SValue& v) :type(t), value(v){}

	void SetType(EValueType t) { type = t; }
	EValueType GetType() const { return type; }
	string GetTypeString() const {
		string ret;
		if (IsFunction() || IsNativeFunction()) ret = "function";
		else if (IsBoolean()) ret = "boolean";
		else if (IsInteger()) ret = "integer";
		else if (IsFloat()) ret = "float";
		else if (IsString()) ret = "string";
		else if (IsLightUData()) ret = "lightudata";
		else if (IsTable()) ret = "table";
		else if (IsCoroutine()) ret = "coroutine";
		else if (IsNull()) ret = "null";

		return ret;
	}
	void SetBool(bool b) { type = EValueType::EBOOLEAN; value.bValue = b; value.cl = nullptr; }
	void SetInt(int i) { type = EValueType::EINTEGER; value.iValue = i;  value.cl = nullptr; }
	void SetFloat(float f) { type = EValueType::EFLOAT; value.fValue = f; value.cl = nullptr; }
	void SetString(string s) { type = EValueType::ESTRING; value.sValue = s; value.cl = nullptr; }
	void SetFunction(Clptr cl) { type = EValueType::EFUNC; value.cl = cl; }
	void SetNativeFunction(SFunc f) { type = EValueType::ENATIVEFUNC; value.sfunc = f; value.cl = nullptr; }
	void SetLightUData(int i) { type = EValueType::ELIGHTUDATA; value.iValue = i; value.cl = nullptr; }
	void SetTable(int i) { type = EValueType::ETABLE; value.iValue = i; value.cl = nullptr; }
	void SetCoroutine(Coroutine* co) { type = EValueType::ECOROUTINE; value.co = co; value.cl = nullptr; }
	void SetNull() { type = EValueType::ENULL; value.cl = nullptr; }
	bool IsBoolean() const { return type == EValueType::EBOOLEAN; }
	bool IsInteger() const { return type == EValueType::EINTEGER; }
	bool IsFloat() const { return type == EValueType::EFLOAT; }
	bool IsNumber() const { return type & EValueType::ENUMBER; }
	bool IsString() const { return type == EValueType::ESTRING; }
	bool IsFunction() const { return type == EValueType::EFUNC; }
	bool IsNativeFunction() const { return type == EValueType::ENATIVEFUNC; }
	bool IsLightUData() const { return type == EValueType::ELIGHTUDATA; }
	bool IsTable() const { return type == EValueType::ETABLE; }
	bool IsCoroutine() const { return type == EValueType::ECOROUTINE; }
	bool IsNull() const { return type == EValueType::ENULL; }
	bool GetBoolean() const { return value.bValue; }
	int GetInteger() const { return value.iValue; }
	float GetFloat() const { return value.fValue; }
	string GetString() const { return value.sValue; }
	Clptr& GetFunction() { return value.cl; }
	SFunc GetNativeFunction() const { return value.sfunc; }
	int GetLightUData() const { return value.iValue; }
	int GetTable() const { return value.iValue; }
	Coroutine* GetCoroutine() const { return value.co; }

	Value operator-(){
		Value va;
		if (!IsNumber()){
			Error::GetInstance()->ProcessError("尝试对非Number值进行求负操作\n");
			va.SetBool(false);
			return va;
		}

		if (IsInteger()){
			va.SetInt(-value.iValue);
		}
		else{
			va.SetFloat(-value.fValue);
		}

		return va;
	}

	Value operator+(Value& v){
		Value va;
		if (IsString() && v.IsString()){
			va.SetString(value.sValue + v.GetString());
			return va;
		}
		if (!IsNumber() || !v.IsNumber()){
			Error::GetInstance()->ProcessError("尝试对非Number值进行加操作\n");
			va.SetBool(false);
			return va;
		}
		if (type == v.GetType()){
			if (IsInteger()){
				va.SetInt(value.iValue + v.GetInteger());
			}
			else if (IsFloat()){
				va.SetFloat(value.fValue + v.GetFloat());
			}

			return va;
		}
		else{
			float a, b;
			if (IsInteger()) a = value.iValue;
			else a = value.fValue;
			if (v.IsInteger()) b = v.value.iValue;
			else b = v.value.fValue;
			va.SetFloat(a + b);
			return va;
		}
	}

	Value operator-(Value& v){
		Value va;
		if (!IsNumber() || !v.IsNumber()){
			Error::GetInstance()->ProcessError("尝试对非Number值进行减操作\n");
			va.SetBool(false);
			return va;
		}
		if (type == v.GetType()){
			if (IsInteger()){
				va.SetInt(value.iValue - v.GetInteger());
			}
			else if (IsFloat()){
				va.SetFloat(value.fValue - v.GetFloat());
			}

			return va;
		}
		else{
			float a, b;
			if (IsInteger()) a = value.iValue;
			else a = value.fValue;
			if (v.IsInteger()) b = v.value.iValue;
			else b = v.value.fValue;
			va.SetFloat(a - b);
			return va;
		}
	}

	Value operator*(Value& v){
		Value va;
		if (!IsNumber() || !v.IsNumber()){
			Error::GetInstance()->ProcessError("尝试对非Number值进行乘操作\n");
			va.SetBool(false);
			return va;
		}
		if (type == v.GetType()){
			if (IsInteger()){
				va.SetInt(value.iValue * v.GetInteger());
			}
			else if (IsFloat()){
				va.SetFloat(value.fValue * v.GetFloat());
			}

			return va;
		}
		else{
			float a, b;
			if (IsInteger()) a = value.iValue;
			else a = value.fValue;
			if (v.IsInteger()) b = v.value.iValue;
			else b = v.value.fValue;
			va.SetFloat(a * b);
			return va;
		}
	}

	Value operator/(Value& v){
		Value va;
		if (!IsNumber() || !v.IsNumber()){
			Error::GetInstance()->ProcessError("尝试对非Number值进行除操作\n");
			va.SetBool(false);
			return va;
		}
		if (type == v.GetType()){
			if (IsInteger()){
				va.SetInt(value.iValue / v.GetInteger());
			}
			else if (IsFloat()){
				va.SetFloat(value.fValue / v.GetFloat());
			}

			return va;
		}
		else{
			float a, b;
			if (IsInteger()) a = value.iValue;
			else a = value.fValue;
			if (v.IsInteger()) b = v.value.iValue;
			else b = v.value.fValue;
			va.SetFloat(a / b);
			return va;
		}
	}

	Value operator%(Value& v){
		Value va;
		if (!IsInteger() || !v.IsInteger()){
			Error::GetInstance()->ProcessError("尝试对非Integer值取模\n");
			va.SetBool(false);
			return va;
		}

		va.SetInt(value.iValue % v.GetInteger());
		return va;
	}

	Value operator+=(Value& v){
		SValue sv;
		if (!IsNumber() || !v.IsNumber()){
			Error::GetInstance()->ProcessError("尝试对非Number值进行加操作\n");
			this->SetBool(false);
			return *this;
		}
		if (type == v.GetType()){
			if (IsInteger()){
				value.iValue += v.GetInteger();
			}
			else if (IsFloat()){
				value.fValue += v.GetFloat();
			}

			return *this;
		}
		else{
			float a, b;
			if (IsInteger()) a = value.iValue;
			else a = value.fValue;
			if (v.IsInteger()) b = v.value.iValue;
			else b = v.value.fValue;
			this->SetFloat(a + b);
			return *this;
		}
	}

	Value operator-=(Value& v){
		SValue sv;
		if (!IsNumber() || !v.IsNumber()){
			Error::GetInstance()->ProcessError("尝试对非Number值进行减操作\n");
			this->SetBool(false);
			return *this;
		}
		if (type == v.GetType()){
			if (IsInteger()){
				value.iValue -= v.GetInteger();
			}
			else if (IsFloat()){
				value.fValue -= v.GetFloat();
			}

			return *this;
		}
		else{
			float a, b;
			if (IsInteger()) a = value.iValue;
			else a = value.fValue;
			if (v.IsInteger()) b = v.value.iValue;
			else b = v.value.fValue;
			this->SetFloat(a - b);
			return *this;
		}
	}

	Value operator*=(Value& v){
		SValue sv;
		if (!IsNumber() || !v.IsNumber()){
			Error::GetInstance()->ProcessError("尝试对非Number值进行乘操作\n");
			this->SetBool(false);
			return *this;
		}
		if (type == v.GetType()){
			if (IsInteger()){
				value.iValue *= v.GetInteger();
			}
			else if (IsFloat()){
				value.fValue *= v.GetFloat();
			}

			return *this;
		}
		else{
			float a, b;
			if (IsInteger()) a = value.iValue;
			else a = value.fValue;
			if (v.IsInteger()) b = v.value.iValue;
			else b = v.value.fValue;
			this->SetFloat(a * b);
			return *this;
		}
	}

	Value operator/=(Value& v){
		SValue sv;
		if (!IsNumber() || !v.IsNumber()){
			Error::GetInstance()->ProcessError("尝试对非Number值进行除操作\n");
			this->SetBool(false);
			return *this;
		}
		if (type == v.GetType()){
			if (IsInteger()){
				value.iValue /= v.GetInteger();
			}
			else if (IsFloat()){
				value.fValue /= v.GetFloat();
			}

			return *this;
		}
		else{
			float a, b;
			if (IsInteger()) a = value.iValue;
			else a = value.fValue;
			if (v.IsInteger()) b = v.value.iValue;
			else b = v.value.fValue;
			this->SetFloat(a / b);
			return *this;
		}
	}

	Value operator%=(Value& v){
		SValue sv;
		if (!IsInteger() || !IsInteger()){
			Error::GetInstance()->ProcessError("尝试对非Integer值取模\n");
			this->SetBool(false);
			return *this;
		}

		value.iValue %= v.GetInteger();
		return *this;
	}

	Value operator<(Value& v){
		Value va;
		if (!IsNumber() || !v.IsNumber()){
			Error::GetInstance()->ProcessError("尝试比较非Number值\n");
			va.SetBool(false);
		}
		else{
			float a, b;
			if (IsInteger()) a = value.iValue;
			else a = value.fValue;
			if (v.IsInteger()) b = v.value.iValue;
			else b = v.value.fValue;
			va.SetBool(a < b);
		}

		return va;
	}

	Value operator>(Value& v){
		Value va;
		if (!IsNumber() || !v.IsNumber()){
			Error::GetInstance()->ProcessError("尝试比较非Number值\n");
			va.SetBool(false);
		}
		else{
			float a, b;
			if (IsInteger()) a = value.iValue;
			else a = value.fValue;
			if (v.IsInteger()) b = v.value.iValue;
			else b = v.value.fValue;
			va.SetBool(a > b);
		}

		return va;
	}

	Value operator<=(Value& v){
		Value va;
		if (!IsNumber() || !v.IsNumber()){
			Error::GetInstance()->ProcessError("尝试比较非Number值\n");
			va.SetBool(false);
		}
		else{
			float a, b;
			if (IsInteger()) a = value.iValue;
			else a = value.fValue;
			if (v.IsInteger()) b = v.value.iValue;
			else b = v.value.fValue;
			va.SetBool(a <= b);
		}

		return va;
	}

	Value operator>=(Value& v){
		Value va;
		if (!IsNumber() || !v.IsNumber()){
			Error::GetInstance()->ProcessError("尝试比较非Number值\n");
			va.SetBool(false);
		}
		else{
			float a, b;
			if (IsInteger()) a = value.iValue;
			else a = value.fValue;
			if (v.IsInteger()) b = v.value.iValue;
			else b = v.value.fValue;
			va.SetBool(a >= b);
		}

		return va;
	}

	Value operator==(Value& v){
		Value va;
		if (IsNumber() && v.IsNumber()){
			float a, b;
			if (IsInteger()) a = value.iValue;
			else a = value.fValue;
			if (v.IsInteger()) b = v.value.iValue;
			else b = v.value.fValue;
			va.SetBool(a == b);
			return va;
		}

		if (type != v.GetType()){
			va.SetBool(false);
		}
		else {
			if (IsBoolean()){
				va.SetBool(value.bValue == v.value.bValue);
			}
			else if (IsString()){
				va.SetBool(value.sValue == v.value.sValue);
			}
			else if (IsNativeFunction()){
				va.SetBool(value.sfunc == v.value.sfunc);
			}
		}

		return va;
	}

	Value operator!=(Value& v){
		Value va;
		if (IsNumber() && v.IsNumber()){
			float a, b;
			if (IsInteger()) a = value.iValue;
			else a = value.fValue;
			if (v.IsInteger()) b = v.value.iValue;
			else b = v.value.fValue;
			va.SetBool(a != b);
			return va;
		}

		if (type != v.GetType()){
			va.SetBool(true);
		}
		else {
			if (IsBoolean()){
				va.SetBool(value.bValue != v.value.bValue);
			}
			else if (IsString()){
				va.SetBool(value.sValue != v.value.sValue);
			}
			else if (IsNativeFunction()){
				va.SetBool(value.sfunc != v.value.sfunc);
			}
		}

		return va;
	}

	Value operator||(Value& v){
		bool t1, t2;
		if (IsFloat()) t1 = GetFloat() != 0;
		else t1 = GetBoolean();
		if (v.IsFloat()) t2 = v.GetFloat() != 0;
		else t2 = v.GetBoolean();

		Value ret;
		ret.SetBool(t1 || t2);
		return ret;
	}

	Value operator&&(Value& v){
		bool t1, t2;
		if (IsFloat()) t1 = GetFloat() != 0;
		else t1 = GetBoolean();
		if (v.IsFloat()) t2 = v.GetFloat() != 0;
		else t2 = v.GetBoolean();

		Value ret;
		ret.SetBool(t1 && t2);
		return ret;
	}

	string ToString(){
		string ret;
		if (IsBoolean()){
			if (value.bValue) ret = "true";
			else ret = "false";
		}
		else if (IsInteger()){
			ret = to_string(value.iValue);
		}
		else if (IsFloat()){
			ret = to_string(value.fValue);
		}
		else if (IsString()){
			ret = value.sValue;
		}
		else if (IsLightUData()){
			ret = to_string(value.iValue);
		}
		else if (IsTable()){
			ret = "table";
		}
		else if (IsCoroutine()){
			ret = "coroutine";
		}
		else if (IsNull()){
			ret = "null";
		}
		else{
			ret = "function";
		}

		return ret;
	}
};

struct Closure{
	int entry;

	//在该函数体内定义的变量
	typedef hash_map<string, Value>::iterator VariableIterator;
	hash_map<string, Value> variables;
	//closure values or non local variables
	vector<int> cvs; //函数生命周期之内的非局部变量
	hash_map<string, Value> ocvs; //函数生命周期之外的非局部变量
};

SABER_NAMESPACE_END

#endif