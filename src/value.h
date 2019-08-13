#ifndef H_VALUE_H
#define H_VALUE_H

#include "common.h"

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
class Value;
class VM;
typedef Value(*SFunc)(VM* vm);
struct SValue{
	union{
		bool   bValue;
		int    iValue;
		float  fValue;
	};
	string sValue;
	shared_ptr<Astree> func;
	SFunc sfunc;
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
	void SetValue(SValue& v) { value = v; }
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
	shared_ptr<Astree> GetFunction() const { return value.func; }
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
		SValue sv;
		Value va;
		va.SetType(EValueType::EBOOLEAN);

		if (type == v.GetType()){
			if (IsInteger()){
				sv.bValue = value.iValue < v.value.iValue;
			}
			else if (IsFloat()){
				sv.bValue = value.fValue < v.value.fValue;
			}
		}
		va.SetValue(sv);
		return va;
	}

	Value operator>(Value& v){
		SValue sv;
		Value va;
		va.SetType(EValueType::EBOOLEAN);

		if (type == v.GetType()){
			if (IsInteger()){
				sv.bValue = value.iValue > v.value.iValue;
			}
			else if (IsFloat()){
				sv.bValue = value.fValue > v.value.fValue;
			}
		}
		va.SetValue(sv);
		return va;
	}

	Value operator<=(Value& v){
		SValue sv;
		Value va;
		va.SetType(EValueType::EBOOLEAN);

		if (type == v.GetType()){
			if (IsInteger()){
				sv.bValue = value.iValue <= v.value.iValue;
			}
			else if (IsFloat()){
				sv.bValue = value.fValue <= v.value.fValue;
			}
		}
		va.SetValue(sv);
		return va;
	}

	Value operator>=(Value& v){
		SValue sv;
		Value va;
		va.SetType(EValueType::EBOOLEAN);

		if (type == v.GetType()){
			if (IsInteger()){
				sv.bValue = value.iValue >= v.value.iValue;
			}
			else if (IsFloat()){
				sv.bValue = value.fValue >= v.value.fValue;
			}
		}
		va.SetValue(sv);
		return va;
	}

	Value operator==(Value& v){
		SValue sv;
		Value va;
		va.SetType(EValueType::EBOOLEAN);

		if (type == v.GetType()){
			if (IsInteger()){
				sv.bValue = value.iValue == v.value.iValue;
			}
			else if (IsFloat()){
				sv.bValue = value.fValue == v.value.fValue;
			}
			else if (IsString()){
				sv.bValue = value.sValue == v.value.sValue;
			}
		}
		va.SetValue(sv);
		return va;
	}

	Value operator!=(Value& v){
		SValue sv;
		Value va;
		va.SetType(EValueType::EBOOLEAN);

		if (type == v.GetType()){
			if (IsInteger()){
				sv.bValue = value.iValue != v.value.iValue;
			}
			else if (IsFloat()){
				sv.bValue = value.fValue != v.value.fValue;
			}
			else if (IsString()){
				sv.bValue = value.sValue != v.value.sValue;
			}
		}
		va.SetValue(sv);
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

#endif