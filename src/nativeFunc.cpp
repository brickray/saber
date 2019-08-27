#include "nativeFunc.h"
#include "sstate.h"
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

void checkFunction(string func, Value& v, int idx = 1){
	if (!v.IsFunction()){
		Error::GetInstance()->ProcessError("%s函数第%d个参数类型需为function\n", func.c_str(), idx);
	}
}

void checkNativeFunction(string func, Value& v, int idx = 1){
	if (!v.IsNativeFunction()){
		Error::GetInstance()->ProcessError("%s函数第%d个参数类型需为nativefunction\n", func.c_str(), idx);
	}
}

void checkLightUData(string func, Value& v, int idx = 1){
	if (!v.IsLightUData()){
		Error::GetInstance()->ProcessError("%s函数第%d个参数类型需为lightudata\n", func.c_str(), idx);
	}
}

void checkTable(string func, Value& v, int idx = 1){
	if (!v.IsTable()){
		Error::GetInstance()->ProcessError("%s函数第%d个参数类型需为table\n", func.c_str(), idx);
	}
}

//-----------------------------------basic lib--------------------
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

static int type(SVM* svm, int numParams){
	checkParamsNum("type", numParams, 1);

	Value v = svm->PopStack();

	svm->PushString(v.GetTypeString());

	return numParams;
}

static int load(SVM* svm, int numParams){
	checkParamsNum("load", numParams);
	Value str = svm->PopStack();
	checkString("load", str);

	SVM::Instruction nop(Opcode::NOP);
	int idx = svm->AddCode(nop);
	SState* S = svm->GetSState();
	S->GetLexer()->Parse(str.GetString());
	S->GetParser()->Parse(*S->GetLexer());
	shared_ptr<Environment> e = S->GetEnvironment();
	shared_ptr<Environment> local = shared_ptr<Environment>(new Environment());
	local->SetOutter(e);
	S->GetParser()->Compile(local, S->GetSVM());
	SVM::Instruction ret(Opcode::RET, 0);
	SVM::Instruction last = svm->GetLastCode();
	if (last.opcode != Opcode::RET){
		svm->AddCode(ret);
	}

	Value func;
	func.SetFunction(idx);
	svm->PushStack(func);

	return numParams;
}

static int test(SVM* svm, int numParams){
	if (numParams == 1){
		svm->CallScript(0);
	}
	else{
		Value v, s;
		v.SetNativeFunction(print);
		s.SetString("it's just a test");

		svm->PushStack(s);
		svm->PushStack(v);
		svm->CallScript(1);
	}

	return numParams;
}

//------------------------------------math lib---------------------
static int sin(SVM* svm, int numParams){
	checkParamsNum("math.sin", numParams);
	Value v = svm->PopStack();
	checkNumber("math.sin", v);

	float ret;
	if (v.IsFloat()) ret = sinf(DegreeToRadians(v.GetFloat()));
	else if (v.IsInteger()) ret = sinf(DegreeToRadians(v.GetInteger()));

	svm->PushFloat(ret);

	return numParams;
}

static int asin(SVM* svm, int numParams){
	checkParamsNum("math.asin", numParams);
	Value v = svm->PopStack();
	checkNumber("math.asin", v);

	float ret;
	if (v.IsFloat()) ret = asinf(v.GetFloat());
	else if (v.IsInteger()) ret = asinf(v.GetInteger());

	svm->PushFloat(ret);

	return numParams;
}

static int cos(SVM* svm, int numParams){
	checkParamsNum("math.cos", numParams);
	Value v = svm->PopStack();
	checkNumber("math.cos", v);

	float ret;
	if (v.IsFloat()) ret = cosf(DegreeToRadians(v.GetFloat()));
	else if (v.IsInteger()) ret = cosf(DegreeToRadians(v.GetInteger()));

	svm->PushFloat(ret);

	return numParams;
}

static int acos(SVM* svm, int numParams){
	checkParamsNum("math.acos", numParams);
	Value v = svm->PopStack();
	checkNumber("math.acos", v);

	float ret;
	if (v.IsFloat()) ret = acosf(v.GetFloat());
	else if (v.IsInteger()) ret = acosf(v.GetInteger());

	svm->PushFloat(ret);

	return numParams;
}

static int tan(SVM* svm, int numParams){
	checkParamsNum("math.tan", numParams);
	Value v = svm->PopStack();
	checkNumber("math.tan", v);

	float ret;
	if (v.IsFloat()) ret = tanf(DegreeToRadians(v.GetFloat()));
	else if (v.IsInteger()) ret = tanf(DegreeToRadians(v.GetInteger()));

	svm->PushFloat(ret);

	return numParams;
}

static int atan(SVM* svm, int numParams){
	checkParamsNum("math.atan", numParams);
	Value v = svm->PopStack();
	checkNumber("math.atan", v);

	float ret;
	if (v.IsFloat()) ret = atanf(v.GetFloat());
	else if (v.IsInteger()) ret = atanf(v.GetInteger());

	svm->PushFloat(ret);

	return numParams;
}

static int radians(SVM* svm, int numParams){
	checkParamsNum("math.radians", numParams);
	Value v = svm->PopStack();
	checkNumber("math.radians", v);

	float ret;
	if (v.IsFloat()) ret = DegreeToRadians(v.GetFloat());
	else if (v.IsInteger()) ret = DegreeToRadians(v.GetInteger());

	svm->PushFloat(ret);

	return numParams;
}

static int degree(SVM* svm, int numParams){
	checkParamsNum("math.degree", numParams);
	Value v = svm->PopStack();
	checkNumber("math.degree", v);

	float ret;
	if (v.IsFloat()) ret = RadiansToDegree(v.GetFloat());
	else if (v.IsInteger()) ret = RadiansToDegree(v.GetInteger());

	svm->PushFloat(ret);

	return numParams;
}

static int abs(SVM* svm, int numParams){
	checkParamsNum("math.abs", numParams);
	Value v = svm->PopStack();
	checkNumber("math.abs", v);

	float ret;
	if (v.IsFloat()) ret = fabsf(v.GetFloat());
	else if (v.IsInteger()) ret = fabsf(v.GetInteger());

	svm->PushFloat(ret);

	return numParams;
}

static int log(SVM* svm, int numParams){
	checkParamsNum("math.log", numParams);
	Value v = svm->PopStack();
	checkNumber("math.log", v);

	float ret;
	if (v.IsFloat()) ret = logf(v.GetFloat());
	else if (v.IsInteger()) ret = logf(v.GetInteger());

	svm->PushFloat(ret);

	return numParams;
}

static int exp(SVM* svm, int numParams){
	checkParamsNum("math.exp", numParams);
	Value v = svm->PopStack();
	checkNumber("math.exp", v);

	float ret;
	if (v.IsFloat()) ret = expf(v.GetFloat());
	else if (v.IsInteger()) ret = expf(v.GetInteger());

	svm->PushFloat(ret);

	return numParams;
}

static int sqrt(SVM* svm, int numParams){
	checkParamsNum("math.sqrt", numParams);
	Value v = svm->PopStack();
	checkNumber("math.sqrt", v);

	float ret;
	if (v.IsFloat()) ret = sqrtf(v.GetFloat());
	else if (v.IsInteger()) ret = sqrtf(v.GetInteger());

	svm->PushFloat(ret);

	return numParams;
}

static int pow(SVM* svm, int numParams){
	checkParamsNum("math.pow", numParams, 2);
	Value exponent = svm->PopStack();
	Value base = svm->PopStack();
	checkNumber("math.pow", base);
	checkNumber("math.pow", exponent, 2);

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
	checkParamsNum("os.gettime", numParams, 0);

	time_t t = time(nullptr);
	string str = ctime(&t);
	//去掉末尾的换行符
	str = str.substr(0, str.size() - 1);
	svm->PushString(str);

	return numParams;
}

static int getclock(SVM* svm, int numParams){
	checkParamsNum("os.getclock", numParams, 0);
	
	clock_t t = clock();
	t /= CLOCKS_PER_SEC;

	svm->PushInt(t);

	return numParams;
}

static int osexit(SVM* svm, int numParams){
	checkParamsNum("os.exit", numParams, 0);

	exit(0);
	
	return numParams;
}

static int osleep(SVM* svm, int numParams){
	checkParamsNum("os.sleep", numParams, 1);
	Value v = svm->PopStack();
	checkInteger("os.sleep", v);

	int t = v.GetInteger();
#if WIN32
	Sleep(t);
#endif

	return numParams;
}

//------------------------------str lib------------------------
static int len(SVM* svm, int numParams){
	checkParamsNum("string.len", numParams);
	Value str = svm->PopStack();
	checkString("string.len", str);

	svm->PushInt(str.GetString().length());

	return numParams;
}

static int substr(SVM* svm, int numParams){
	checkParamsNum("string.substr", numParams, 3);
	Value endV = svm->PopStack();
	Value startV = svm->PopStack();
	Value strV = svm->PopStack();
	checkString("string.substr", strV);
	checkInteger("string.substr", startV, 2);
	checkInteger("string.substr", endV, 3);

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

		checkString("string.findfirst", strV);
		checkString("string.findfirst", findV, 2);
	}
	else if (numParams == 3){
		startV = svm->PopStack();
		findV = svm->PopStack();
		strV = svm->PopStack();

		checkString("string.findfirst", strV);
		checkString("string.findfirst", findV, 2);
		checkInteger("string.findfirst", startV, 3);
	}
	else{
		Error::GetInstance()->ProcessError("string.findfirst函数只接收2个或3个参数\n");
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

		checkString("string.findlast", strV);
		checkString("string.findlast", findV, 2);
	}
	else if (numParams == 3){
		startV = svm->PopStack();
		findV = svm->PopStack();
		strV = svm->PopStack();

		checkString("string.findlast", strV);
		checkString("string.findlast", findV, 2);
		checkInteger("string.findlast", startV, 3);
	}
	else{
		Error::GetInstance()->ProcessError("string.findlast函数只接收2个或3个参数\n");
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

		checkString("string.findsub", strV);
		checkString("string.findsub", findV, 2);
	}
	else if (numParams == 3){
		startV = svm->PopStack();
		findV = svm->PopStack();
		strV = svm->PopStack();

		checkString("string.findsub", strV);
		checkString("string.findsub", findV, 2);
		checkInteger("string.findsub", startV, 3);
	}
	else{
		Error::GetInstance()->ProcessError("string.findsub函数只接收2个或3个参数\n");
	}

	string str = strV.GetString();
	string find = findV.GetString();
	int start = 0;
	if (numParams == 3) start = startV.GetInteger();

	svm->PushInt(str.find(find, start));

	return numParams;
}

static int insert(SVM* svm, int numParams){
	checkParamsNum("string.insert", numParams, 3);
	Value pV = svm->PopStack();
	Value insertV = svm->PopStack();
	Value strV = svm->PopStack();
	checkString("string.insert", strV);
	checkString("string.insert", insertV, 2);
	checkInteger("string.insert", pV, 3);

	string str = strV.GetString();
	string i = insertV.GetString();
	int p0 = pV.GetInteger();
	
	svm->PushString(str.insert(p0, i));

	return numParams;
}

static int reverse(SVM* svm, int numParams){
	checkParamsNum("string.reverse", numParams, 1);
	Value strV = svm->PopStack();
	checkString("string.reverse", strV);

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

static int sforeach(SVM* svm, int numParams){
	checkParamsNum("string.foreach", numParams, 2);
	Value str, func;
	func = svm->PopStack();
	str = svm->PopStack();
	checkString("string.foreach", str);
	checkFunction("string.foreach", func, 2);

	string s = str.GetString();
	char t[2] = { 0 };
	for (int i = 0; i < s.size(); ++i){
		Value idx, value;
		idx.SetInt(i);
		t[0] = s[i];
		value.SetString(t);

		svm->PushStack(idx);
		svm->PushStack(value);
		svm->PushStack(func);
		svm->CallScript(2);
	}
	
	return numParams;
}

//------------------------------io lib-------------------------
static int fexist(SVM* svm, int numParams){
	Value fileV, modeV;
	checkParamsNum("io.fexist", numParams);
	fileV = svm->PopStack();
	checkString("io.fexist", fileV);

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

		checkString("io.open", fileV);
	}
	else if (numParams == 2){
		modeV = svm->PopStack();
		fileV = svm->PopStack();

		checkString("io.open", fileV);
		checkString("io.open", modeV, 2);
	}
	else{
		Error::GetInstance()->ProcessError("io.open函数只接收1个或2个参数\n");
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
	checkParamsNum("io.read", numParams);
	file = svm->PopStack();
	checkLightUData("io.read", file);

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
	checkParamsNum("io.write", numParams, 2);
	data = svm->PopStack();
	file = svm->PopStack();
	checkLightUData("io.write", file);

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
	checkParamsNum("io.close", numParams);
	file = svm->PopStack();
	checkLightUData("io.close", file);

	FILE* f = reinterpret_cast<FILE*>(file.GetLightUData());
	fclose(f);

	return numParams;
}

//-------------------------------table lib----------------------
static int remove(SVM* svm, int numParams){
	checkParamsNum("table.remove", numParams, 2);
	Value s = svm->PopStack();
	Value t = svm->PopStack();
	checkTable("table.remove", t);
	checkString("table.remove", s, 2);

	Table* table = reinterpret_cast<Table*>(t.GetTable());
	TableIteration ti = table->kv.find(s.GetString());
	if (ti != table->kv.end()) 
		table->kv.erase(ti);

	return numParams;
}

static int tlength(SVM* svm, int numParams){
	checkParamsNum("table.len", numParams);
	Value t = svm->PopStack();
	checkTable("table.len", t);

	Table* table = reinterpret_cast<Table*>(t.GetTable());
	svm->PushInt(table->kv.size());

	return numParams;
}

static int tforeach(SVM* svm, int numParams){
	checkParamsNum("table.foreach", numParams, 2);
	Value table, func;
	func = svm->PopStack();
	table = svm->PopStack();
	checkTable("table.foreach", table);
	checkFunction("table.foreach", func, 2);

	Table* t = reinterpret_cast<Table*>(table.GetTable());
	TableIteration ti = t->kv.begin();
	for (; ti != t->kv.end(); ++ti){
		Value key, value;
		key.SetString(ti->first);
		value = ti->second;

		svm->PushStack(key);
		svm->PushStack(value);
		svm->PushStack(func);
		svm->CallScript(2);
	}

	return numParams;
}

//------------------------------func register-------------------

static RegisterFunction basic[] = {
	{ "print", print },
	{ "type", type },
	{ "load", load },
//	{ "test", test },
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
	{ "foreach", sforeach },
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

static RegisterFunction tb[] = {
	{ "remove", remove },
	{ "len", tlength },
	{ "foreach", tforeach },
	{ "", nullptr },
};

void registerTable(shared_ptr<Environment>& e, shared_ptr<SVM>& svm){
	Value table;
	Table* t = new Table();
	table.SetTable((int)t);
	int idx = svm->AddGlobal(table);
	SymbolInfo si = { table, idx };
	e->SetSymbol("table", si);
	for (int i = 0;; ++i){
		string name = tb[i].name;
		if (name == "") break;

		Value function;
		function.SetNativeFunction(tb[i].f);
		t->kv[name] = function;
	}
}

void NativeFunc::Register(shared_ptr<Environment>& e, shared_ptr<SVM>& svm){
	registerBasic(e, svm);
	registerMath(e, svm);
	registerOs(e, svm);
	registerStr(e, svm);
	registerIo(e, svm);
	registerTable(e, svm);
}

SABER_NAMESPACE_END