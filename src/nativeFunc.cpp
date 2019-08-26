#include "nativeFunc.h"
#include "astree.h"
#include "error.h"

#include <time.h>
#include <stdio.h>
#if WIN32
#include <windows.h>
#endif

SABER_NAMESPACE_BEGIN

void checkParamsNum(string func, int numParams, int expect = 1){
	if (numParams != expect){
		Error::GetInstance()->ProcessError("%s函数只接收%d个参数\n", func.c_str(), expect);
	}

	return;
}

void checkBoolean(string func, Value& v, int idx = 1){
	if (!v.IsBoolean()){
		Error::GetInstance()->ProcessError("%s函数第%d个参数类型需为boolean\n", func.c_str(), idx);
	}
}

void checkNumber(string func, Value& v, int idx = 1){
	if (!v.IsNumber()){
		Error::GetInstance()->ProcessError("%s函数第%d个参数类型需为number\n", func.c_str(), idx);
	}
}

void checkInteger(string func, Value& v, int idx = 1){
	if (!v.IsInteger()){
		Error::GetInstance()->ProcessError("%s函数第%d个参数类型需为integer\n", func.c_str(), idx);
	}
}

void checkFloat(string func, Value& v, int idx = 1){
	if (!v.IsFloat()){
		Error::GetInstance()->ProcessError("%s函数第%d个参数类型需为float\n", func.c_str(), idx);
	}
}

void checkString(string func, Value& v, int idx = 1){
	if (!v.IsString()){
		Error::GetInstance()->ProcessError("%s函数第%d个参数类型需为string\n", func.c_str(), idx);
	}
}

void checkLightUData(string func, Value& v, int idx = 1){
	if (!v.IsLightUData()){
		Error::GetInstance()->ProcessError("%s函数第%d个参数类型需为lightudata\n", func.c_str(), idx);
	}
}

static int print(SVM* svm, int numParams){
	string ret;
	vector<string> temp(numParams);
	for (int i = 0; i < numParams; ++i){
		temp[i] = svm->PopStack().ToString();
	}
	for (int i = numParams - 1; i >= 0; --i){
		ret += temp[i];
		ret += "  ";
	}

	printf("%s\n", ret.c_str());

	return numParams;
}

//------------------------------------math lib---------------------
static int sin(SVM* svm, int numParams){
	checkParamsNum("sin", numParams);
	Value v = svm->PopStack();
	checkNumber("sin", v);

	float ret;
	if (v.IsFloat()) ret = sinf(DegreeToRadians(v.GetFloat()));
	else if (v.IsInteger()) ret = sinf(DegreeToRadians(v.GetInteger()));

	svm->PushFloat(ret);

	return numParams;
}

static int asin(SVM* svm, int numParams){
	checkParamsNum("asin", numParams);
	Value v = svm->PopStack();
	checkNumber("asin", v);

	float ret;
	if (v.IsFloat()) ret = asinf(v.GetFloat());
	else if (v.IsInteger()) ret = asinf(v.GetInteger());

	svm->PushFloat(ret);

	return numParams;
}

static int cos(SVM* svm, int numParams){
	checkParamsNum("cos", numParams);
	Value v = svm->PopStack();
	checkNumber("cos", v);

	float ret;
	if (v.IsFloat()) ret = cosf(DegreeToRadians(v.GetFloat()));
	else if (v.IsInteger()) ret = cosf(DegreeToRadians(v.GetInteger()));

	svm->PushFloat(ret);

	return numParams;
}

static int acos(SVM* svm, int numParams){
	checkParamsNum("acos", numParams);
	Value v = svm->PopStack();
	checkNumber("acos", v);

	float ret;
	if (v.IsFloat()) ret = acosf(v.GetFloat());
	else if (v.IsInteger()) ret = acosf(v.GetInteger());

	svm->PushFloat(ret);

	return numParams;
}

static int tan(SVM* svm, int numParams){
	checkParamsNum("tan", numParams);
	Value v = svm->PopStack();
	checkNumber("tan", v);

	float ret;
	if (v.IsFloat()) ret = tanf(DegreeToRadians(v.GetFloat()));
	else if (v.IsInteger()) ret = tanf(DegreeToRadians(v.GetInteger()));

	svm->PushFloat(ret);

	return numParams;
}

static int atan(SVM* svm, int numParams){
	checkParamsNum("atan", numParams);
	Value v = svm->PopStack();
	checkNumber("atan", v);

	float ret;
	if (v.IsFloat()) ret = atanf(v.GetFloat());
	else if (v.IsInteger()) ret = atanf(v.GetInteger());

	svm->PushFloat(ret);

	return numParams;
}

static int radians(SVM* svm, int numParams){
	checkParamsNum("radians", numParams);
	Value v = svm->PopStack();
	checkNumber("radians", v);

	float ret;
	if (v.IsFloat()) ret = DegreeToRadians(v.GetFloat());
	else if (v.IsInteger()) ret = DegreeToRadians(v.GetInteger());

	svm->PushFloat(ret);

	return numParams;
}

static int degree(SVM* svm, int numParams){
	checkParamsNum("degree", numParams);
	Value v = svm->PopStack();
	checkNumber("degree", v);

	float ret;
	if (v.IsFloat()) ret = RadiansToDegree(v.GetFloat());
	else if (v.IsInteger()) ret = RadiansToDegree(v.GetInteger());

	svm->PushFloat(ret);

	return numParams;
}

static int abs(SVM* svm, int numParams){
	checkParamsNum("abs", numParams);
	Value v = svm->PopStack();
	checkNumber("abs", v);

	float ret;
	if (v.IsFloat()) ret = fabsf(v.GetFloat());
	else if (v.IsInteger()) ret = fabsf(v.GetInteger());

	svm->PushFloat(ret);

	return numParams;
}

static int log(SVM* svm, int numParams){
	checkParamsNum("log", numParams);
	Value v = svm->PopStack();
	checkNumber("log", v);

	float ret;
	if (v.IsFloat()) ret = logf(v.GetFloat());
	else if (v.IsInteger()) ret = logf(v.GetInteger());

	svm->PushFloat(ret);

	return numParams;
}

static int exp(SVM* svm, int numParams){
	checkParamsNum("exp", numParams);
	Value v = svm->PopStack();
	checkNumber("exp", v);

	float ret;
	if (v.IsFloat()) ret = expf(v.GetFloat());
	else if (v.IsInteger()) ret = expf(v.GetInteger());

	svm->PushFloat(ret);

	return numParams;
}

static int sqrt(SVM* svm, int numParams){
	checkParamsNum("sqrt", numParams);
	Value v = svm->PopStack();
	checkNumber("sqrt", v);

	float ret;
	if (v.IsFloat()) ret = sqrtf(v.GetFloat());
	else if (v.IsInteger()) ret = sqrtf(v.GetInteger());

	svm->PushFloat(ret);

	return numParams;
}

static int pow(SVM* svm, int numParams){
	checkParamsNum("pow", numParams, 2);
	Value exponent = svm->PopStack();
	Value base = svm->PopStack();
	checkNumber("pow", base);
	checkNumber("pow", exponent, 2);

	float b = 1, e = 1;
	if (base.IsFloat()) b = base.GetFloat();
	else if (base.IsInteger()) b = base.GetInteger();
	if (exponent.IsFloat()) e = exponent.GetFloat();
	else if (exponent.IsInteger()) e = exponent.GetInteger();

	float ret = powf(b, e);
	svm->PushFloat(ret);

	return numParams;
}

//---------------------------------os lib-------------------------
static int gettime(SVM* svm, int numParams){
	checkParamsNum("gettime", numParams, 0);

	time_t t = time(nullptr);
	string str = ctime(&t);
	//去掉末尾的换行符
	str = str.substr(0, str.size() - 1);
	svm->PushString(str);

	return numParams;
}

static int getclock(SVM* svm, int numParams){
	checkParamsNum("getclock", numParams, 0);
	
	clock_t t = clock();
	t /= CLOCKS_PER_SEC;

	svm->PushInt(t);

	return numParams;
}

static int osexit(SVM* svm, int numParams){
	checkParamsNum("exit", numParams, 0);

	exit(0);
	
	return numParams;
}

static int osleep(SVM* svm, int numParams){
	checkParamsNum("sleep", numParams, 1);
	Value v = svm->PopStack();
	checkInteger("sleep", v);

	int t = v.GetInteger();
#if WIN32
	Sleep(t);
#endif

	return numParams;
}

static int type(SVM* svm, int numParams){
	checkParamsNum("type", numParams, 1);

	Value v = svm->PopStack();
	
	svm->PushString(v.GetTypeString());

	return numParams;
}

//------------------------------str lib------------------------
static int len(SVM* svm, int numParams){
	checkParamsNum("len", numParams);
	Value str = svm->PopStack();
	checkString("len", str);

	svm->PushInt(str.GetString().length());

	return numParams;
}

static int substr(SVM* svm, int numParams){
	checkParamsNum("substr", numParams, 3);
	Value endV = svm->PopStack();
	Value startV = svm->PopStack();
	Value strV = svm->PopStack();
	checkString("substr", strV);
	checkInteger("substr", startV, 2);
	checkInteger("substr", endV, 3);

	int start = startV.GetInteger();
	int end = endV.GetInteger();
	int length = end - start + 1;
	string str = strV.GetString();
	svm->PushString(str.substr(start, length));

	return numParams;
}

static int findfirst(SVM* svm, int numParams){
	Value strV, findV, startV;
	if (numParams == 2){
		findV = svm->PopStack();
		strV = svm->PopStack();

		checkString("findfirst", strV);
		checkString("findfirst", findV, 2);
	}
	else if (numParams == 3){
		startV = svm->PopStack();
		findV = svm->PopStack();
		strV = svm->PopStack();

		checkString("findfirst", strV);
		checkString("findfirst", findV, 2);
		checkInteger("findfirst", startV, 3);
	}
	else{
		Error::GetInstance()->ProcessError("findfirst函数只接收2个或3个参数\n");
	}

	string str = strV.GetString();
	string find = findV.GetString();
	int start = 0;
	if (numParams == 3) start = startV.GetInteger();

	svm->PushInt(str.find_first_of(find, start));

	return numParams;
}

static int findlast(SVM* svm, int numParams){
	Value strV, findV, startV;
	if (numParams == 2){
		findV = svm->PopStack();
		strV = svm->PopStack();

		checkString("findlast", strV);
		checkString("findlast", findV, 2);
	}
	else if (numParams == 3){
		startV = svm->PopStack();
		findV = svm->PopStack();
		strV = svm->PopStack();

		checkString("findlast", strV);
		checkString("findlast", findV, 2);
		checkInteger("findlast", startV, 3);
	}
	else{
		Error::GetInstance()->ProcessError("findlast函数只接收2个或3个参数\n");
	}

	string str = strV.GetString();
	string find = findV.GetString();
	int start = str.length();
	if (numParams == 3) start = startV.GetInteger();

	svm->PushInt(str.find_last_of(find, start));

	return numParams;
}

static int findsub(SVM* svm, int numParams){
	Value strV, findV, startV;
	if (numParams == 2){
		findV = svm->PopStack();
		strV = svm->PopStack();

		checkString("findsub", strV);
		checkString("findsub", findV, 2);
	}
	else if (numParams == 3){
		startV = svm->PopStack();
		findV = svm->PopStack();
		strV = svm->PopStack();

		checkString("findsub", strV);
		checkString("findsub", findV, 2);
		checkInteger("findsub", startV, 3);
	}
	else{
		Error::GetInstance()->ProcessError("findsub函数只接收2个或3个参数\n");
	}

	string str = strV.GetString();
	string find = findV.GetString();
	int start = 0;
	if (numParams == 3) start = startV.GetInteger();

	svm->PushInt(str.find(find, start));

	return numParams;
}

static int insert(SVM* svm, int numParams){
	checkParamsNum("insert", numParams, 3);
	Value pV = svm->PopStack();
	Value insertV = svm->PopStack();
	Value strV = svm->PopStack();
	checkString("insert", strV);
	checkString("insert", insertV, 2);
	checkInteger("insert", pV, 3);

	string str = strV.GetString();
	string i = insertV.GetString();
	int p0 = pV.GetInteger();
	
	svm->PushString(str.insert(p0, i));

	return numParams;
}

static int reverse(SVM* svm, int numParams){
	checkParamsNum("reverse", numParams, 1);
	Value strV = svm->PopStack();
	checkString("reverse", strV);

	string str = strV.GetString();
	string ret;
	ret.resize(str.length());
	int size = str.length();
	for (int i = 0; i < size; ++i){
		ret[i] = str[size - i - 1];
	}
	
	svm->PushString(ret);

	return numParams;
}

//------------------------------io lib-------------------------
static int fexist(SVM* svm, int numParams){
	Value fileV, modeV;
	checkParamsNum("fexist", numParams);
	fileV = svm->PopStack();
	checkString("fexist", fileV);

	string file = fileV.GetString();
	string mode = "r";

	FILE* f = nullptr;
	f = fopen(file.c_str(), mode.c_str());
	if (f) svm->PushBool(true);
	else svm->PushBool(false);

	return numParams;
}

static int open(SVM* svm, int numParams){
	Value fileV, modeV;
	if (numParams == 1){
		fileV = svm->PopStack();

		checkString("open", fileV);
	}
	else if (numParams == 2){
		modeV = svm->PopStack();
		fileV = svm->PopStack();

		checkString("open", fileV);
		checkString("open", modeV, 2);
	}
	else{
		Error::GetInstance()->ProcessError("open函数只接收1个或2个参数\n");
	}

	string file = fileV.GetString();
	string mode = "r";
	if (numParams == 2) mode = modeV.GetString();

	FILE* f = nullptr;
	f = fopen(file.c_str(), mode.c_str());
	
	svm->PushLightUData(int(f));

	return numParams;
}

static int read(SVM* svm, int numParams){
	Value file;
	checkParamsNum("read", numParams);
	file = svm->PopStack();
	checkLightUData("read", file);

	FILE* f = reinterpret_cast<FILE*>(file.GetLightUData());
	fseek(f, 0, SEEK_END);
	long length = ftell(f);
	rewind(f);
	char* str = new char[length + 1];
	int result = fread(str, 1, length, f);
	str[result] = '\0';
	svm->PushString(str);
	delete[] str;

	return numParams;
}

static int write(SVM* svm, int numParams){
	Value file, data;
	checkParamsNum("write", numParams, 2);
	data = svm->PopStack();
	file = svm->PopStack();
	checkLightUData("write", file);

	FILE* f = reinterpret_cast<FILE*>(file.GetLightUData());
	if (data.IsBoolean()){
		bool b = data.GetBoolean();
		fwrite(&b, sizeof(bool), 1, f);
	}
	else if (data.IsInteger()){
		int i = data.GetInteger();
		fwrite(&i, sizeof(int), 1, f);
	}
	else if (data.IsFloat()){
		float fl = data.GetFloat();
		fwrite(&fl, sizeof(float), 1, f);
	}
	else if (data.IsString()){
		string s = data.GetString(); 
		fwrite(s.c_str(), s.length()*sizeof(char), 1, f);
	}

	return numParams;
}

static int close(SVM* svm, int numParams){
	Value file;
	checkParamsNum("close", numParams);
	file = svm->PopStack();
	checkLightUData("close", file);

	FILE* f = reinterpret_cast<FILE*>(file.GetLightUData());
	fclose(f);

	return numParams;
}

static RegisterFunction basic[] = {
	{ "print", print },
	{ "type", type },
	{ "", nullptr },
};

void registerBasic(shared_ptr<Environment>& e, shared_ptr<SVM>& svm){
	for (int i = 0;; ++i){
		string name = basic[i].name;
		if (name == "") break;

		Value function;
		function.SetNativeFunction(basic[i].f);
		int idx = svm->AddGlobal(function);
		SymbolInfo si = { function, idx };
		e->SetSymbol(name, si);
	}
}

static RegisterFunction math[] = {
	{ "sin", sin },
	{ "asin", asin },
	{ "cos", cos },
	{ "acos", acos },
	{ "tan", tan },
	{ "atan", atan },
	{ "radians", radians },
	{ "degree", degree },
	{ "abs", abs },
	{ "log", log },
	{ "exp", exp },
	{ "sqrt", sqrt },
	{ "pow", pow },
	{ "", nullptr },
};

void registerMath(shared_ptr<Environment>& e, shared_ptr<SVM>& svm){
	Value table;
	Table* t = new Table();
	table.SetTable((int)t);
	int idx = svm->AddGlobal(table);
	SymbolInfo si = { table, idx };
	e->SetSymbol("math", si);
	for (int i = 0;; ++i){
		string name = math[i].name;
		if (name == "") break;

		Value function;
		function.SetNativeFunction(math[i].f);
		t->kv[name] = function;
	}
}

static RegisterFunction os[] = {
	{ "gettime", gettime },
	{ "getclock", getclock },
	{ "exit", osexit },
	{ "sleep", osleep },
	{ "", nullptr },
};

void registerOs(shared_ptr<Environment>& e, shared_ptr<SVM>& svm){
	Value table;
	Table* t = new Table();
	table.SetTable((int)t);
	int idx = svm->AddGlobal(table);
	SymbolInfo si = { table, idx };
	e->SetSymbol("os", si);
	for (int i = 0;; ++i){
		string name = os[i].name;
		if (name == "") break;

		Value function;
		function.SetNativeFunction(os[i].f);
		t->kv[name] = function;
	}
}

static RegisterFunction str[] = {
	{ "len", len },
	{ "substr", substr },
	{ "findfirst", findfirst },
	{ "findlast", findlast },
	{ "findsub", findsub },
	{ "insert", insert },
	{ "reverse", reverse },
	{ "", nullptr },
};

void registerStr(shared_ptr<Environment>& e, shared_ptr<SVM>& svm){
	Value table;
	Table* t = new Table();
	table.SetTable((int)t);
	int idx = svm->AddGlobal(table);
	SymbolInfo si = { table, idx };
	e->SetSymbol("string", si);
	for (int i = 0;; ++i){
		string name = str[i].name;
		if (name == "") break;

		Value function;
		function.SetNativeFunction(str[i].f);
		t->kv[name] = function;
	}
}

static RegisterFunction io[] = {
	{ "fexist", fexist },
	{ "open", open },
	{ "read", read },
	{ "write", write },
	{ "close", close },
	{ "", nullptr },
};

void registerIo(shared_ptr<Environment>& e, shared_ptr<SVM>& svm){
	Value table;
	Table* t = new Table();
	table.SetTable((int)t);
	int idx = svm->AddGlobal(table);
	SymbolInfo si = { table, idx };
	e->SetSymbol("io", si);
	for (int i = 0;; ++i){
		string name = io[i].name;
		if (name == "") break;

		Value function;
		function.SetNativeFunction(io[i].f);
		t->kv[name] = function;
	}
}

void NativeFunc::Register(shared_ptr<Environment>& e, shared_ptr<SVM>& svm){
	registerBasic(e, svm);
	registerMath(e, svm);
	registerOs(e, svm);
	registerStr(e, svm);
	registerIo(e, svm);
}

SABER_NAMESPACE_END