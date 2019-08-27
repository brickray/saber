#ifndef H_VALUE_H
#define H_VALUE_H

#include "common.h"
#include "error.h"

SABER_NAMESPACE_BEGIN

enum EValueType{
	EBOOLEAN    = 1,
	EINTEGER    = 2,
	EFLOAT      = 4,
	ESTRING     = 8,
	EFUNC       = 16,
	ENATIVEFUNC = 32,
	ELIGHTUDATA = 64,
	ETABLE      = 128,
	ENULL       = 256,
	ENUMBER     = EINTEGER | EFLOAT,
};

class Environment;
class Astree;
class SVM;
typedef int(*SFunc)(SVM* vm, int numParams);
struct SValue{
	union{
		bool   bValue;
		int    iValue;
		float  fValue;
		SFunc sfunc;
	};
	string sValue;
};

class Value;
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
		else if (IsNull()) ret = "null";

		return ret;
	}
	void SetBool(bool b) { type = EValueType::EBOOLEAN; value.bValue = b; }
	void SetInt(int i) { type = EValueType::EINTEGER; value.iValue = i; }
	void SetFloat(float f) { type = EValueType::EFLOAT; value.fValue = f; }
	void SetString(string s) { type = EValueType::ESTRING; value.sValue = s; }
	void SetFunction(int i) { type = EValueType::EFUNC; value.iValue = i; }
	void SetNativeFunction(SFunc f) { type = EValueType::ENATIVEFUNC; value.sfunc = f; }
	void SetLightUData(int i) { type = EValueType::ELIGHTUDATA; value.iValue = i; }
	void SetTable(int i) { type = EValueType::ETABLE; value.iValue = i; }
	bool IsBoolean() const { return type == EValueType::EBOOLEAN; }
	bool IsInteger() const { return type == EValueType::EINTEGER; }
	bool IsFloat() const { return type == EValueType::EFLOAT; }
	bool IsNumber() const { return type & EValueType::ENUMBER; }
	bool IsString() const { return type == EValueType::ESTRING; }
	bool IsFunction() const { return type == EValueType::EFUNC; }
	bool IsNativeFunction() const { return type == EValueType::ENATIVEFUNC; }
	bool IsLightUData() const { return type == EValueType::ELIGHTUDATA; }
	bool IsTable() const { return type == EValueType::ETABLE; }
	bool IsNull() const { return type == EValueType::ENULL; }
	bool GetBoolean() const { return value.bValue; }
	int GetInteger() const { return value.iValue; }
	float GetFloat() const { return value.fValue; }
	string GetString() const { return value.sValue; }
	int GetFunction() const { return value.iValue; }
	SFunc GetNativeFunction() const { return value.sfunc; }
	int GetLightUData() const { return value.iValue; }
	int GetTable() const { return value.iValue; }

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
		else if (IsNull()){
			ret = "null";
		}
		else{
			ret = "function";
		}

		return ret;
	}
};

SABER_NAMESPACE_END

#endif