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
struct Table;
struct Coroutine;
typedef shared_ptr<Closure> Clptr;
typedef shared_ptr<Table> Tptr;
typedef shared_ptr<Coroutine> Coptr;
struct Coroutine{
	ECoroutineStatus status;
	Clptr cl;
	int ip;
};

struct SValue{
	union{
		bool       bValue;
		Integer        iValue;
		Float      fValue;
		SFunc      sfunc;
	};
	Tptr   t;
	Clptr  cl;
	Coptr  co;
	string sValue;

	SValue(){
		t = nullptr;
		cl = nullptr;
		co = nullptr;
	}

	~SValue(){
		if(t) t = nullptr;
		if(cl) cl = nullptr;
		if(co) co = nullptr;
	}
};

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
	void SetBool(bool b) { type = EValueType::EBOOLEAN; value.bValue = b; value.cl = nullptr; value.t = nullptr; }
	void SetInt(Integer i) { type = EValueType::EINTEGER; value.iValue = i;  value.cl = nullptr; value.t = nullptr; }
	void SetFloat(Float f) { type = EValueType::EFLOAT; value.fValue = f; value.cl = nullptr; value.t = nullptr; }
	void SetString(string s) { type = EValueType::ESTRING; value.sValue = s; value.cl = nullptr; value.t = nullptr; }
	void SetFunction(Clptr cl) { type = EValueType::EFUNC; value.cl = cl; value.t = nullptr; }
	void SetNativeFunction(SFunc f) { type = EValueType::ENATIVEFUNC; value.sfunc = f; value.cl = nullptr; value.t = nullptr; }
	void SetLightUData(Integer i) { type = EValueType::ELIGHTUDATA; value.iValue = i; value.cl = nullptr; value.t = nullptr; }
	void SetTable(Tptr t) { type = EValueType::ETABLE; value.t = t; value.cl = nullptr; }
	void SetCoroutine(Coptr co) { type = EValueType::ECOROUTINE; value.co = co; value.cl = nullptr; value.t = nullptr; }
	void SetNull() { type = EValueType::ENULL; value.cl = nullptr; value.t = nullptr; }
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
	Integer GetInteger() const { return value.iValue; }
	Float GetFloat() const { return value.fValue; }
	string GetString() const { return value.sValue; }
	Clptr& GetFunction() { return value.cl; }
	SFunc GetNativeFunction() const { return value.sfunc; }
	Integer GetLightUData() const { return value.iValue; }
	Tptr& GetTable() { return value.t; }
	Coptr& GetCoroutine() { return value.co; }

	Value& operator=(Value& v){
		type = v.type;
		if (v.IsString()){
			value.sValue = v.value.sValue;
		}
		else if (v.IsTable()){
			value.t = v.value.t;
		}
		else if (v.IsFunction()){
			value.cl = v.value.cl;
		}
		else{
			value.bValue = v.value.bValue;
			value.iValue = v.value.iValue;
			value.fValue = v.value.fValue;
			value.sfunc = v.value.sfunc;
			value.co = v.value.co;
		}

		return *this;
	}

	Value operator-(){
		Value va;
		if (!IsNumber()){
			Error::GetInstance()->ProcessError("尝试对非Number值进行求负操作[%s]", GetTypeString().c_str());
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
			Error::GetInstance()->ProcessError("尝试对非Number值进行加操作[%s, %s]", GetTypeString().c_str(), v.GetTypeString().c_str());
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
			Float a, b;
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
			Error::GetInstance()->ProcessError("尝试对非Number值进行减操作[%s, %s]", GetTypeString().c_str(), v.GetTypeString().c_str());
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
			Float a, b;
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
			Error::GetInstance()->ProcessError("尝试对非Number值进行乘操作[%s, %s]", GetTypeString().c_str(), v.GetTypeString().c_str());
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
			Float a, b;
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
			Error::GetInstance()->ProcessError("尝试对非Number值进行除操作[%s, %s]", GetTypeString().c_str(), v.GetTypeString().c_str());
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
			Float a, b;
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
			Error::GetInstance()->ProcessError("尝试对非Integer值取模[%s, %s]", GetTypeString().c_str(), v.GetTypeString().c_str());
			va.SetBool(false);
			return va;
		}

		va.SetInt(value.iValue % v.GetInteger());
		return va;
	}

	Value operator+=(Value& v){
		if (IsString() && v.IsString()){
			value.sValue += v.GetString();
			return *this;
		}

		if (!IsNumber() || !v.IsNumber()){
			Error::GetInstance()->ProcessError("尝试对非Number值进行加操作[%s, %s]", GetTypeString().c_str(), v.GetTypeString().c_str());
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
			Float a, b;
			if (IsInteger()) a = value.iValue;
			else a = value.fValue;
			if (v.IsInteger()) b = v.value.iValue;
			else b = v.value.fValue;
			this->SetFloat(a + b);
			return *this;
		}
	}

	Value operator-=(Value& v){
		if (!IsNumber() || !v.IsNumber()){
			Error::GetInstance()->ProcessError("尝试对非Number值进行减操作[%s, %s]", GetTypeString().c_str(), v.GetTypeString().c_str());
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
			Float a, b;
			if (IsInteger()) a = value.iValue;
			else a = value.fValue;
			if (v.IsInteger()) b = v.value.iValue;
			else b = v.value.fValue;
			this->SetFloat(a - b);
			return *this;
		}
	}

	Value operator*=(Value& v){
		if (!IsNumber() || !v.IsNumber()){
			Error::GetInstance()->ProcessError("尝试对非Number值进行乘操作[%s, %s]", GetTypeString().c_str(), v.GetTypeString().c_str());
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
			Float a, b;
			if (IsInteger()) a = value.iValue;
			else a = value.fValue;
			if (v.IsInteger()) b = v.value.iValue;
			else b = v.value.fValue;
			this->SetFloat(a * b);
			return *this;
		}
	}

	Value operator/=(Value& v){
		if (!IsNumber() || !v.IsNumber()){
			Error::GetInstance()->ProcessError("尝试对非Number值进行除操作[%s, %s]", GetTypeString().c_str(), v.GetTypeString().c_str());
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
			Float a, b;
			if (IsInteger()) a = value.iValue;
			else a = value.fValue;
			if (v.IsInteger()) b = v.value.iValue;
			else b = v.value.fValue;
			this->SetFloat(a / b);
			return *this;
		}
	}

	Value operator%=(Value& v){
		if (!IsInteger() || !IsInteger()){
			Error::GetInstance()->ProcessError("尝试对非Integer值取模[%s, %s]", GetTypeString().c_str(), v.GetTypeString().c_str());
			this->SetBool(false);
			return *this;
		}

		value.iValue %= v.GetInteger();
		return *this;
	}

	Value operator<(Value& v){
		Value va;
		if (!IsNumber() || !v.IsNumber()){
			Error::GetInstance()->ProcessError("尝试比较非Number值[%s, %s]", GetTypeString().c_str(), v.GetTypeString().c_str());
			va.SetBool(false);
		}
		else{
			Float a, b;
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
			Error::GetInstance()->ProcessError("尝试比较非Number值[%s, %s]", GetTypeString().c_str(), v.GetTypeString().c_str());
			va.SetBool(false);
		}
		else{
			Float a, b;
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
			Error::GetInstance()->ProcessError("尝试比较非Number值[%s, %s]", GetTypeString().c_str(), v.GetTypeString().c_str());
			va.SetBool(false);
		}
		else{
			Float a, b;
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
			Error::GetInstance()->ProcessError("尝试比较非Number值[%s, %s]", GetTypeString().c_str(), v.GetTypeString().c_str());
			va.SetBool(false);
		}
		else{
			Float a, b;
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
			Float a, b;
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
			if (IsNull()){
				va.SetBool(true);
			}
			if (IsBoolean()){
				va.SetBool(value.bValue == v.value.bValue);
			}
			else if (IsString()){
				va.SetBool(value.sValue == v.value.sValue);
			}
			else if (IsNativeFunction()){
				va.SetBool(value.sfunc == v.value.sfunc);
			}
			else if (IsLightUData()){
				va.SetBool(value.iValue == v.value.iValue);
			}
			else if (IsTable()){
				va.SetBool(value.t == v.value.t);
			}
			else if (IsCoroutine()){
				va.SetBool(value.cl == v.value.cl);
			}
		}

		return va;
	}

	Value operator!=(Value& v){
		Value va;
		if (IsNumber() && v.IsNumber()){
			Float a, b;
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
			if (IsNull()){
				va.SetBool(false);
			}
			if (IsBoolean()){
				va.SetBool(value.bValue != v.value.bValue);
			}
			else if (IsString()){
				va.SetBool(value.sValue != v.value.sValue);
			}
			else if (IsNativeFunction()){
				va.SetBool(value.sfunc != v.value.sfunc);
			}
			else if (IsLightUData()){
				va.SetBool(value.iValue != v.value.iValue);
			}
			else if (IsTable()){
				va.SetBool(value.t != v.value.t);
			}
			else if (IsCoroutine()){
				va.SetBool(value.cl != v.value.cl);
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

typedef hash_map<string, Value>::iterator TableIteration;
struct Table{
private:
	hash_map<string, Value> kv;

public:
	void AddBool(string key, bool b) { Value v; v.SetBool(b); kv[key] = v; }
	void AddInt(string key, Integer i) { Value v; v.SetInt(i); kv[key] = v; }
	void AddFloat(string key, Float f) { Value v; v.SetFloat(f); kv[key] = v; }
	void AddString(string key, string s) { Value v; v.SetString(s); kv[key] = v; }
	void AddLightUData(string key, void* p) { Value v; v.SetLightUData(Integer(p)); kv[key] = v; }
	void AddFunction(string key, Clptr cl) { Value v; v.SetFunction(cl); kv[key] = v; }
	void AddNativeFunction(string key, SFunc f) { Value v; v.SetNativeFunction(f); kv[key] = v; }
	void AddTable(string key, Tptr t) { Value v; v.SetTable(t); kv[key] = v; }
	void AddCoroutine(string key, Coptr co) { Value v; v.SetCoroutine(co); kv[key] = v; }
	void AddValue(string key, Value& v) { kv[key] = v; }

	Value GetValue(string key) {
		if (kv.find(key) != kv.end()){
			return kv[key];
		}

		return Value();
	}

	//key must exist
	Value* GetValuePtr(string key){
		return &kv[key];
	}

	bool HasValue(string key){
		return kv.find(key) != kv.end();
	}

	void Remove(string key){
		TableIteration ti = kv.find(key);
		if (ti != kv.end())
			kv.erase(ti);
	}

	int GetLength() const{
		return kv.size();
	}

	TableIteration Begin(){
		return kv.begin();
	}

	TableIteration End(){
		return kv.end();
	}
};

struct Closure{
	bool closure; //是否时新建的闭包
	bool hascv;   //是否有非局部变量 
	bool vararg;  //是否可变参
	int entry;    //入口地址
	int fp;       //形参数量

	hash_map<string, int> variables; //在该函数体内定义的变量
	hash_map<string, Value> cvs; //非局部变量
	hash_set<Clptr> cls; //该函数内定义的函数
};

SABER_NAMESPACE_END

#endif