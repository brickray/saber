#ifndef H_VALUE_H
#define H_VALUE_H

#include "common.h"

SABER_NAMESPACE_BEGIN

enum EValueType{
	EBOOLEAN,
	EINTEGER,
	EFLOAT,
	ESTRING,
	EFUNC,
	ENATIVEFUNC,
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

class Value{
private:
	SValue value;
	EValueType type;

public:
	Value(){}
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

		return ret;
	}
	void SetBool(bool b) { type = EValueType::EBOOLEAN; value.bValue = b; }
	void SetInt(int i) { type = EValueType::EINTEGER; value.iValue = i; }
	void SetFloat(float f) { type = EValueType::EFLOAT; value.fValue = f; }
	void SetString(string s) { type = EValueType::ESTRING; value.sValue = s; }
	bool IsBoolean() const { return type == EValueType::EBOOLEAN; }
	bool IsInteger() const { return type == EValueType::EINTEGER; }
	bool IsFloat() const { return type == EValueType::EFLOAT; }
	bool IsString() const { return type == EValueType::ESTRING; }
	bool IsFunction() const { return type == EValueType::EFUNC; }
	bool IsNativeFunction() const { return type == EValueType::ENATIVEFUNC; }
	bool GetBoolean() const { return value.bValue; }
	int GetInteger() const { return value.iValue; }
	float GetFloat() const { return value.fValue; }
	string GetString() const { return value.sValue; }
	SFunc GetNativeFunction() const { return value.sfunc; }

	Value operator+(Value& v){
		SValue sv;
		Value value = *this;
		if (type == v.GetType()){
			if (IsInteger()){
				value.value.iValue += v.GetInteger();
			}
			else if (IsFloat()){
				value.value.fValue += v.GetFloat();
			}
			else if (IsString()){
				value.value.sValue += v.GetString();
			}

			return value;
		}
	}

	Value operator-(Value& v){
		SValue sv;
		Value value = *this;
		if (type == v.GetType()){
			if (IsInteger()){
				value.value.iValue -= v.GetInteger();
			}
			else if (IsFloat()){
				value.value.fValue -= v.GetFloat();
			}

			return value;
		}
	}

	Value operator*(Value& v){
		SValue sv;
		Value value = *this;
		if (type == v.GetType()){
			if (IsInteger()){
				value.value.iValue *= v.GetInteger();
			}
			else if (IsFloat()){
				value.value.fValue *= v.GetFloat();
			}

			return value;
		}
	}

	Value operator/(Value& v){
		SValue sv;
		Value value = *this;
		if (type == v.GetType()){
			if (IsInteger()){
				value.value.iValue /= v.GetInteger();
			}
			else if (IsFloat()){
				value.value.fValue /= v.GetFloat();
			}

			return value;
		}
	}

	Value operator%(Value& v){
		SValue sv;
		Value value = *this;
		if (type == v.GetType()){
			if (IsInteger()){
				value.value.iValue %= v.GetInteger();
			}

			return value;
		}
	}

	Value operator+=(Value& v){
		SValue sv;
		if (type == v.GetType()){
			if (IsInteger()){
				value.iValue += v.GetInteger();
			}
			else if (IsFloat()){
				value.fValue += v.GetFloat();
			}
			else if (IsString()){
				value.sValue += v.GetString();
			}

			return *this;
		}
	}

	Value operator-=(Value& v){
		SValue sv;
		if (type == v.GetType()){
			if (IsInteger()){
				value.iValue -= v.GetInteger();
			}
			else if (IsFloat()){
				value.fValue -= v.GetFloat();
			}

			return *this;
		}
	}

	Value operator*=(Value& v){
		SValue sv;
		if (type == v.GetType()){
			if (IsInteger()){
				value.iValue *= v.GetInteger();
			}
			else if (IsFloat()){
				value.fValue *= v.GetFloat();
			}

			return *this;
		}
	}

	Value operator/=(Value& v){
		SValue sv;
		if (type == v.GetType()){
			if (IsInteger()){
				value.iValue /= v.GetInteger();
			}
			else if (IsFloat()){
				value.fValue /= v.GetFloat();
			}

			return *this;
		}
	}

	Value operator%=(Value& v){
		SValue sv;
		if (type == v.GetType()){
			if (IsInteger()){
				value.iValue %= v.GetInteger();
			}

			return *this;
		}
	}

	Value operator<(Value& v){
		Value va;
		if (type == v.GetType()){
			if (IsInteger()){
				va.SetBool(value.iValue < v.value.iValue);
			}
			else if (IsFloat()){
				va.SetBool(value.fValue < v.value.fValue);
			}
		}

		return va;
	}

	Value operator>(Value& v){
		Value va;
		if (type == v.GetType()){
			if (IsInteger()){
				va.SetBool(value.iValue > v.value.iValue);
			}
			else if (IsFloat()){
				va.SetBool(value.fValue > v.value.fValue);
			}
		}

		return va;
	}

	Value operator<=(Value& v){
		Value va;
		if (type == v.GetType()){
			if (IsInteger()){
				va.SetBool(value.iValue <= v.value.iValue);
			}
			else if (IsFloat()){
				va.SetBool(value.fValue <= v.value.fValue);
			}
		}

		return va;
	}

	Value operator>=(Value& v){
		Value va;
		if (type == v.GetType()){
			if (IsInteger()){
				va.SetBool(value.iValue >= v.value.iValue);
			}
			else if (IsFloat()){
				va.SetBool(value.fValue >= v.value.fValue);
			}
		}

		return va;
	}

	Value operator==(Value& v){
		Value va;
		if (type == v.GetType()){
			if (IsInteger()){
				va.SetBool(value.iValue == v.value.iValue);
			}
			else if (IsFloat()){
				va.SetBool(value.fValue == v.value.fValue);
			}
			else if (IsString()){
				va.SetBool(value.sValue == v.value.sValue);
			}
		}

		return va;
	}

	Value operator!=(Value& v){
		Value va;
		if (type == v.GetType()){
			if (IsInteger()){
				va.SetBool(value.iValue != v.value.iValue);
			}
			else if (IsFloat()){
				va.SetBool(value.fValue != v.value.fValue);
			}
			else if (IsString()){
				va.SetBool(value.sValue != v.value.sValue);
			}
		}

		return va;
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
		else{
			ret = "function";
		}

		return ret;
	}
};

SABER_NAMESPACE_END

#endif