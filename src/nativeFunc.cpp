#include "nativeFunc.h"
#include "sstate.h"
#include "astree.h"
#include "saber.h"

#include <time.h>
#include <stdio.h>
#include <io.h>
#if WIN32
#include <windows.h>
#endif

SABER_NAMESPACE_BEGIN

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

	return 0;
}

static int type(SVM* svm, int numParams){
	checkParamsNum("type", numParams, 1);

	Value v = svm->PopStack();

	svm->PushString(v.GetTypeString());

	return 1;
}

static int seed(SVM* svm, int numParams){
	checkParamsNum("srand", numParams);
	Value v = svm->PopStack();
	checkInteger("srand", v);

	srand(v.GetInteger());

	return 0;
}

static int uniformInt(SVM* svm, int numParams){
	if (numParams != 0 && numParams != 2){
		Error::GetInstance()->ProcessError("uniformInt只接受0个或2个参数");
		svm->PushBool(false);
	}
	Integer s = 0;
	Integer e = 1;
	if (numParams == 2){
		e = svm->PopStack().GetInteger();
		s = svm->PopStack().GetInteger();
	}

	Float r = Float(rand()) / RAND_MAX;
	svm->PushInt((e - s)*r + s);

	return 1;
}

static int uniformFloat(SVM* svm, int numParams){
	if (numParams != 0 && numParams != 2){
		Error::GetInstance()->ProcessError("uniformFloat只接受0个或2个参数");
		svm->PushBool(false);
	}
	Integer s = 0;
	Integer e = 1;
	if (numParams == 2){
		e = svm->PopStack().GetInteger();
		s = svm->PopStack().GetInteger();
	}

	Float r = Float(rand()) / RAND_MAX;
	svm->PushFloat((e - s)*r + s);

	return 1;
}

static int load(SVM* svm, int numParams){
	checkParamsNum("load", numParams);
	Value str = svm->PopStack();
	checkString("load", str);

	SVM::Instruction nop(Opcode::NOP);
	int idx = svm->AddCode(nop);
	svm->RemoveLastCode();
	SState* S = svm->GetSState();
	S->GetLexer()->Parse(str.GetString());
	S->GetParser()->Parse(*S->GetLexer());
	shared_ptr<Environment> e = S->GetEnvironment();
	shared_ptr<Environment> local = shared_ptr<Environment>(new Environment());
	local->SetOutter(e);
	BlockCnt bc;
	bc.variableIndex = NUM_ADDRESS;
	S->GetParser()->Compile(local, S->GetSVM(), bc);
	SVM::Instruction ret(Opcode::RET, 0);
	SVM::Instruction last = svm->GetLastCode();
	if (last.opcode != Opcode::RET){
		svm->AddCode(ret);
	}
	svm->UpdateCodeSize();

	Value func;
	Clptr cl = Clptr(new Closure());
	cl->hascv = false;
	cl->vararg = false;
	cl->entry = idx;
	cl->fp = 0;
	func.SetFunction(cl);
	svm->PushStack(func);

	return 1;
}

static int next(SVM* svm, int numParams){
	checkParamsNum("next", numParams);
	Value v = svm->PopStack();
	if (!v.IsTable()){
		svm->PushBool(false);
		return 1;
	}

	Tptr table = v.GetTable();
	if (table->Foreach()) svm->PushTable(table->GetIterTable());
	else svm->PushBool(false);

	return 1;
}

static int reverseBit(SVM* svm, int numParams){
	checkParamsNum("reverseBit", numParams);
	Value v = svm->PopStack();
	checkInteger("reverseBit", v);

	Integer n = v.GetInteger();
	n = (n << 16) | (n >> 16);
	n = ((n & 0x00ff00ff) << 8) | ((n & 0xff00ff00) >> 8);
	n = ((n & 0x0f0f0f0f) << 4) | ((n & 0xf0f0f0f0) >> 4);
	n = ((n & 0x33333333) << 2) | ((n & 0xcccccccc) >> 2);
	n = ((n & 0x55555555) << 1) | ((n & 0xaaaaaaaa) >> 1);
	
	svm->PushInt(n);

	return 1;
}

static int toint(SVM* svm, int numParams){
	checkParamsNum("toint", numParams);
	Value v = svm->PopStack();
	if (!(v.IsBoolean() || v.IsNumber() || v.IsString())){
		Error::GetInstance()->ProcessError("toint参数类型错误");
	}

	if (v.IsBoolean()){
		if (v.GetBoolean()) svm->PushInt(1);
		else svm->PushInt(0);
	}
	else if (v.IsInteger()){
		svm->PushStack(v);
	}
	else if (v.IsFloat()){
		Float f = v.GetFloat();
		svm->PushInt(f);
	}
	else{
		string str = v.GetString();
		svm->PushInt(atoi(str.c_str()));
	}

	return 1;
}

static int tostring(SVM* svm, int numParams){
	checkParamsNum("tostring", numParams);
	Value v = svm->PopStack();
	if (!(v.IsBoolean() || v.IsNumber() || v.IsString())){
		Error::GetInstance()->ProcessError("tostring参数类型错误");
	}

	if (v.IsBoolean()){
		if (v.GetBoolean()) svm->PushString("true");
		else svm->PushString("false");
	}
	if (v.IsInteger()){
		svm->PushString(to_string(v.GetInteger()));
	}
	else if (v.IsFloat()){
		svm->PushString(to_string(v.GetFloat()));
	}
	else{
		svm->PushStack(v);
	}

	return 1;
}

static int isnull(SVM* svm, int numParams){
	checkParamsNum("isnull", numParams);
	Value v = svm->PopStack();

	svm->PushBool(v.IsNull());

	return 1;
}

static int isbool(SVM* svm, int numParams){
	checkParamsNum("isbool", numParams);
	Value v = svm->PopStack();

	svm->PushBool(v.IsBoolean());

	return 1;
}

static int isint(SVM* svm, int numParams){
	checkParamsNum("isint", numParams);
	Value v = svm->PopStack();
	
	svm->PushBool(v.IsInteger());

	return 1;
}

static int isfloat(SVM* svm, int numParams){
	checkParamsNum("isfloat", numParams);
	Value v = svm->PopStack();

	svm->PushBool(v.IsFloat());

	return 1;
}

static int isnumber(SVM* svm, int numParams){
	checkParamsNum("isnumber", numParams);
	Value v = svm->PopStack();

	svm->PushBool(v.IsNumber());

	return 1;
}

static int isstring(SVM* svm, int numParams){
	checkParamsNum("isstring", numParams);
	Value v = svm->PopStack();

	svm->PushBool(v.IsString());

	return 1;
}

static int islightudata(SVM* svm, int numParams){
	checkParamsNum("islightudata", numParams);
	Value v = svm->PopStack();

	svm->PushBool(v.IsLightUData());

	return 1;
}

static int isfunction(SVM* svm, int numParams){
	checkParamsNum("isfunction", numParams);
	Value v = svm->PopStack();

	svm->PushBool(v.IsFunction());

	return 1;
}

static int isnativefunction(SVM* svm, int numParams){
	checkParamsNum("isnativefunction", numParams);
	Value v = svm->PopStack();

	svm->PushBool(v.IsNativeFunction());

	return 1;
}

static int istable(SVM* svm, int numParams){
	checkParamsNum("istable", numParams);
	Value v = svm->PopStack();

	svm->PushBool(v.IsTable());

	return 1;
}

static int iscoroutine(SVM* svm, int numParams){
	checkParamsNum("iscoroutine", numParams);
	Value v = svm->PopStack();

	svm->PushBool(v.IsCoroutine());

	return 1;
}

static int ispointer(SVM* svm, int numParams){
	checkParamsNum("ispointer", numParams);
	Value v = svm->PopStack();

	svm->PushBool(v.IsPointer());

	return 1;
}

static int getvfrompointer(SVM* svm, int numParams){
	checkParamsNum("getvfrompointer", numParams);
	Value v = svm->PopStack();
	checkPointer("getvfrompointer", v);

	svm->PushStack(*v.GetPointer());

	return 1;
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

	return 0;
}

//------------------------------------math lib---------------------
static int sin(SVM* svm, int numParams){
	checkParamsNum("math.sin", numParams);
	Value v = svm->PopStack();
	checkNumber("math.sin", v);

	Float ret;
	if (v.IsFloat()) ret = sinf(DegreeToRadians(v.GetFloat()));
	else if (v.IsInteger()) ret = sinf(DegreeToRadians(v.GetInteger()));

	svm->PushFloat(ret);

	return 1;
}

static int asin(SVM* svm, int numParams){
	checkParamsNum("math.asin", numParams);
	Value v = svm->PopStack();
	checkNumber("math.asin", v);

	Float ret;
	if (v.IsFloat()) ret = asinf(v.GetFloat());
	else if (v.IsInteger()) ret = asinf(v.GetInteger());

	svm->PushFloat(ret);

	return 1;
}

static int cos(SVM* svm, int numParams){
	checkParamsNum("math.cos", numParams);
	Value v = svm->PopStack();
	checkNumber("math.cos", v);

	Float ret;
	if (v.IsFloat()) ret = cosf(DegreeToRadians(v.GetFloat()));
	else if (v.IsInteger()) ret = cosf(DegreeToRadians(v.GetInteger()));

	svm->PushFloat(ret);

	return 1;
}

static int acos(SVM* svm, int numParams){
	checkParamsNum("math.acos", numParams);
	Value v = svm->PopStack();
	checkNumber("math.acos", v);

	Float ret;
	if (v.IsFloat()) ret = acosf(v.GetFloat());
	else if (v.IsInteger()) ret = acosf(v.GetInteger());

	svm->PushFloat(ret);

	return 1;
}

static int tan(SVM* svm, int numParams){
	checkParamsNum("math.tan", numParams);
	Value v = svm->PopStack();
	checkNumber("math.tan", v);

	Float ret;
	if (v.IsFloat()) ret = tanf(DegreeToRadians(v.GetFloat()));
	else if (v.IsInteger()) ret = tanf(DegreeToRadians(v.GetInteger()));

	svm->PushFloat(ret);

	return 1;
}

static int atan(SVM* svm, int numParams){
	checkParamsNum("math.atan", numParams);
	Value v = svm->PopStack();
	checkNumber("math.atan", v);

	Float ret;
	if (v.IsFloat()) ret = atanf(v.GetFloat());
	else if (v.IsInteger()) ret = atanf(v.GetInteger());

	svm->PushFloat(ret);

	return 1;
}

static int radians(SVM* svm, int numParams){
	checkParamsNum("math.radians", numParams);
	Value v = svm->PopStack();
	checkNumber("math.radians", v);

	Float ret;
	if (v.IsFloat()) ret = DegreeToRadians(v.GetFloat());
	else if (v.IsInteger()) ret = DegreeToRadians(v.GetInteger());

	svm->PushFloat(ret);

	return 1;
}

static int degree(SVM* svm, int numParams){
	checkParamsNum("math.degree", numParams);
	Value v = svm->PopStack();
	checkNumber("math.degree", v);

	Float ret;
	if (v.IsFloat()) ret = RadiansToDegree(v.GetFloat());
	else if (v.IsInteger()) ret = RadiansToDegree(v.GetInteger());

	svm->PushFloat(ret);

	return 1;
}

static int abs(SVM* svm, int numParams){
	checkParamsNum("math.abs", numParams);
	Value v = svm->PopStack();
	checkNumber("math.abs", v);

	Float ret;
	if (v.IsFloat()) ret = fabsf(v.GetFloat());
	else if (v.IsInteger()) ret = fabsf(v.GetInteger());

	svm->PushFloat(ret);

	return 1;
}

static int log(SVM* svm, int numParams){
	checkParamsNum("math.log", numParams);
	Value v = svm->PopStack();
	checkNumber("math.log", v);

	Float ret;
	if (v.IsFloat()) ret = logf(v.GetFloat());
	else if (v.IsInteger()) ret = logf(v.GetInteger());

	svm->PushFloat(ret);

	return 1;
}

static int log2(SVM* svm, int numParams){
	checkParamsNum("math.log2", numParams);
	Value v = svm->PopStack();
	checkNumber("math.log2", v);

	static Float lg2 = logf(2);
	Float ret;
	if (v.IsFloat()) ret = logf(v.GetFloat()) / lg2;
	else if (v.IsInteger()) ret = logf(v.GetInteger()) / lg2;

	svm->PushFloat(ret);

	return 1;
}

static int log10(SVM* svm, int numParams){
	checkParamsNum("math.log10", numParams);
	Value v = svm->PopStack();
	checkNumber("math.log10", v);

	static Float lg10 = logf(10);
	Float ret;
	if (v.IsFloat()) ret = logf(v.GetFloat()) / lg10;
	else if (v.IsInteger()) ret = logf(v.GetInteger()) / lg10;

	svm->PushFloat(ret);

	return 1;
}

static int exp(SVM* svm, int numParams){
	checkParamsNum("math.exp", numParams);
	Value v = svm->PopStack();
	checkNumber("math.exp", v);

	Float ret;
	if (v.IsFloat()) ret = expf(v.GetFloat());
	else if (v.IsInteger()) ret = expf(v.GetInteger());

	svm->PushFloat(ret);

	return 1;
}

static int sqrt(SVM* svm, int numParams){
	checkParamsNum("math.sqrt", numParams);
	Value v = svm->PopStack();
	checkNumber("math.sqrt", v);

	Float ret;
	if (v.IsFloat()) ret = sqrtf(v.GetFloat());
	else if (v.IsInteger()) ret = sqrtf(v.GetInteger());

	svm->PushFloat(ret);

	return 1;
}

static int pow(SVM* svm, int numParams){
	checkParamsNum("math.pow", numParams, 2);
	Value exponent = svm->PopStack();
	Value base = svm->PopStack();
	checkNumber("math.pow", base);
	checkNumber("math.pow", exponent, 2);

	Float b = 1, e = 1;
	if (base.IsFloat()) b = base.GetFloat();
	else if (base.IsInteger()) b = base.GetInteger();
	if (exponent.IsFloat()) e = exponent.GetFloat();
	else if (exponent.IsInteger()) e = exponent.GetInteger();

	Float ret = powf(b, e);
	svm->PushFloat(ret);

	return 1;
}

static int floor(SVM* svm, int numParams){
	checkParamsNum("math.floor", numParams);
	Value n = svm->PopStack();
	checkNumber("math.floor", n);

	if (n.IsInteger()){
		svm->PushStack(n);
	}
	else{
		Float f = n.GetFloat();
		f = floorf(f);
		n.SetFloat(f);
		svm->PushStack(n);
	}

	return 1;
}

static int ceil(SVM* svm, int numParams){
	checkParamsNum("math.ceil", numParams);
	Value n = svm->PopStack();
	checkNumber("math.ceil", n);

	if (n.IsInteger()){
		svm->PushStack(n);
	}
	else{
		Float f = n.GetFloat();
		f = ceilf(f);
		n.SetFloat(f);
		svm->PushStack(n);
	}

	return 1;
}

static int mmax(SVM* svm, int numParams){
	checkParamsNum("math.max", numParams, 2);
	Value v2 = svm->PopStack();
	Value v1 = svm->PopStack();

	v1 = (v1 > v2).GetBoolean() ? v1 : v2;
	svm->PushStack(v1);

	return 1;
}

static int mmin(SVM* svm, int numParams){
	checkParamsNum("math.min", numParams, 2);
	Value v2 = svm->PopStack();
	Value v1 = svm->PopStack();

	v1 = (v1 < v2).GetBoolean() ? v1 : v2;
	svm->PushStack(v1);

	return 1;
}

//---------------------------------os lib-------------------------
static int gettime(SVM* svm, int numParams){
	checkParamsNum("os.gettime", numParams, 0);

	time_t t = time(nullptr);
	string str = ctime(&t);
	//去掉末尾的换行符
	str = str.substr(0, str.size() - 1);
	svm->PushString(str);

	return 1;
}

static int gettimei(SVM* svm, int numParams){
	checkParamsNum("os.time", numParams, 0);

	svm->PushInt(time(nullptr));

	return 1;
}

static int getclock(SVM* svm, int numParams){
	checkParamsNum("os.getclock", numParams, 0);
	
	clock_t t = clock();
	Float c = Float(t) / CLOCKS_PER_SEC;

	svm->PushFloat(c);

	return 1;
}

static int osexit(SVM* svm, int numParams){
	checkParamsNum("os.exit", numParams, 0);

	exit(0);
	
	return 0;
}

static int osleep(SVM* svm, int numParams){
	checkParamsNum("os.sleep", numParams, 1);
	Value v = svm->PopStack();
	checkInteger("os.sleep", v);

	Integer t = v.GetInteger();
#if WIN32
	Sleep(t);
#endif

	return 0;
}

//------------------------------str lib------------------------
static int len(SVM* svm, int numParams){
	checkParamsNum("string.len", numParams);
	Value str = svm->PopStack();
	checkString("string.len", str);

	svm->PushInt(str.GetString().length());

	return 1;
}

static int substr(SVM* svm, int numParams){
	checkParamsNum("string.substr", numParams, 3);
	Value endV = svm->PopStack();
	Value startV = svm->PopStack();
	Value strV = svm->PopStack();
	checkString("string.substr", strV);
	checkInteger("string.substr", startV, 2);
	checkInteger("string.substr", endV, 3);

	Integer start = startV.GetInteger();
	Integer end = endV.GetInteger();
	Integer length = end - start + 1;
	string str = strV.GetString();
	svm->PushString(str.substr(start, length));

	return 1;
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
	Integer start = 0;
	if (numParams == 3) start = startV.GetInteger();

	svm->PushInt(str.find_first_of(find, start));

	return 1;
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
	Integer start = str.length();
	if (numParams == 3) start = startV.GetInteger();

	svm->PushInt(str.find_last_of(find, start));

	return 1;
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
	Integer start = 0;
	if (numParams == 3) start = startV.GetInteger();

	svm->PushInt(str.find(find, start));

	return 1;
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
	Integer p0 = pV.GetInteger();
	
	svm->PushString(str.insert(p0, i));

	return 1;
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

	return 1;
}

static int at(SVM* svm, int numParams){
	checkParamsNum("string.at", numParams, 2);
	Value p = svm->PopStack();
	Value strV = svm->PopStack();
	checkString("string.at", strV);
	checkInteger("string.at", p, 2);

	string str = strV.GetString();
	Integer p0 = p.GetInteger();
	//string的索引为[0, len - 1]
	//string在len处的字符为终结符'\0'
	if (p0 > str.length()){
		Error::GetInstance()->ProcessError("索引[%d]需小于字符串长度[%d]", p0, str.length());
	}

	char t[2] = { 0 };
	t[0] = str[p0];
	svm->PushString(t);

	return 1;
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
	
	return 0;
}

static int format(SVM* svm, int numParams){
	vector<Value> p(numParams);
	for (int i = 0; i < numParams;++i)
		p[numParams - i - 1] = svm->PopStack();
	checkString("string.format", p[0]);

	string s = p[0].GetString();
	string ret;
	int np = 1;
	for (int i = 0; i < s.size(); ++i){
		char c = s[i];
		int precise = 0;
	label:
		if (c == '%'){
			char next = s[++i];
			switch(next){
			case '%':
				ret += '%';
				break;
			case 'c':
			case 'C':{
				Value v = p[np++];
				if (!v.IsInteger()){
					Error::GetInstance()->ProcessError("格式化选项[%%c]需要integer类型的参数");
				}

				char cc = char(v.GetInteger());
				ret += cc;
				break;
			}
			case 'd':{
				Value v = p[np++];
				if (!v.IsInteger()){
					Error::GetInstance()->ProcessError("格式化选项[%%d]需要integer类型的参数");
				}

				char buf[256] = { 0 };
				string f = "%." + to_string(precise) + "d";
				_snprintf(buf, sizeof(buf), f.c_str(), v.GetInteger());
				ret += buf;
				break;
			}
			case 'u':{
				Value v = p[np++];
				if (!v.IsInteger()){
					Error::GetInstance()->ProcessError("格式化选项[%%u]需要integer类型的参数");
				}

				unsigned int i = v.GetInteger();
				char buf[256] = { 0 };
				string f = "%." + to_string(precise) + "u";
				_snprintf(buf, sizeof(buf), f.c_str(), i);
				ret += buf;
				break;
			}
			case 'x':
			case 'X':{
				Value v = p[np++];
				if (!v.IsInteger()){
					Error::GetInstance()->ProcessError("格式化选项[%%x]需要integer类型的参数");
				}

				unsigned int i = v.GetInteger();
				char buf[256] = { 0 };
				string f = "%." + to_string(precise) + next;
				_snprintf(buf, sizeof(buf), f.c_str(), i);

				ret += buf;
				break;
			}
			case 'o':{
				Value v = p[np++];
				if (!v.IsInteger()){
					Error::GetInstance()->ProcessError("格式化选项[%%o]需要integer类型的参数");
				}

				unsigned int i = v.GetInteger();
				char buf[256] = { 0 };
				string f = "%." + to_string(precise) + "o";
				_snprintf(buf, sizeof(buf), f.c_str(), i);
				ret += buf;
				break;
			}
			case 's':{
				Value v = p[np++];
				if (!v.IsString()){
					Error::GetInstance()->ProcessError("格式化选项[%%s]需要string类型的参数");
				}

				ret += v.GetString();
				break;
			}
			case 'e':
			case 'E':{
				Value v = p[np++];
				if (!v.IsNumber()){
					Error::GetInstance()->ProcessError("格式化选项[%%e]需要number类型的参数");
				}

				Float value;
				if (v.IsInteger()) value = v.GetInteger();
				else value = v.GetFloat();
				char buf[256] = { 0 };
				string f = "%." + to_string(precise) + next;
				_snprintf(buf, sizeof(buf), f.c_str(), value);

				ret += buf;
				break;
			}
			case 'g':
			case 'G':{
				Value v = p[np++];
				if (!v.IsNumber()){
					Error::GetInstance()->ProcessError("格式化选项[%%g]需要number类型的参数");
				}

				char buf[256] = { 0 };
				string f = "%." + to_string(precise) + next;
				_snprintf(buf, sizeof(buf), f.c_str(), v.IsInteger() ? v.GetInteger() : v.GetFloat());

				ret += buf;
				break;
			}
			case 'f':{
				Value v = p[np++];
				if (!v.IsNumber()){
					Error::GetInstance()->ProcessError("格式化选项[%%f]需要number类型的参数");
				}

				Float value;
				if (v.IsInteger()) value = v.GetInteger();
				else value = v.GetFloat();

				char buf[256] = { 0 };
				string f = "%." + to_string(precise) + "f";
				_snprintf(buf, sizeof(buf), f.c_str(), value);
				ret += buf;

				break;
			}
			case '.':{
				char number[4] = { 0 };
				char n1 = s[++i];
				if (isdigit(n1)){
					number[0] = n1;
					char n2 = s[++i];
					if (isdigit(n2)){
						number[1] = n2;
					}
					else{
						i--;
					}
				}
				else{
					number[0] = '0';
					i--;
				}

				precise = atoi(number);
				if (precise > 50){
					Error::GetInstance()->ProcessError("格式化精度超过限制最大为50");
				}

				goto label;

				break;
			}
			default:
				Error::GetInstance()->ProcessError("错误的格式化选项[%%%c]", next);
				break;
			}
		}
		else{
			ret += c;
		}
	}

	string t;
	for (int i = 0; i < ret.size(); ++i){
		char c = ret[i];
		if (c == '\\'){
			char next = ret[++i];
			switch (next){
			case 'a':
				t += '\a';
				break;
			case 'b':
				t += '\b';
				break;
			case 'f':
				t += '\f';
				break;
			case 'n':
				t += '\n';
				break;
			case 'r':
				t += '\r';
				break;
			case 't':
				t += '\t';
				break;
			case 'v':
				t += '\v';
				break;
			case '\\':
				t += '\\';
				break;
			case '?':
				t += '\?';
				break;
			case '\"':
				t += '\"';
				break;
			case '\'':
				t += '\'';
				break;
			default:
				t += c;
				t += next;
			}
		}
		else{
			t += c;
		}
	}

	svm->PushString(t);

	return 1;
}

static int lower(SVM* svm, int numParams){
	checkParamsNum("string.lower", numParams);
	Value v = svm->PopStack();
	checkString("string.lower", v);

	string str = v.GetString();
	vector<char> ret(str.size() + 1);
	for (int i = 0; i < str.size(); ++i){
		char c = str[i];
		if (c >= 'A' && c <= 'Z'){
			c += 32;
		}
		ret[i] = c;
	}
	ret[str.size()] = '\0';
	svm->PushString(&ret[0]);

	return 1;
}

static int upper(SVM* svm, int numParams){
	checkParamsNum("string.upper", numParams);
	Value v = svm->PopStack();
	checkString("string.upper", v);

	string str = v.GetString();
	vector<char> ret(str.size() + 1);
	for (int i = 0; i < str.size(); ++i){
		char c = str[i];
		if (c >= 'a' && c <= 'z'){
			c -= 32;
		}
		ret[i] = c;
	}
	ret[str.size()] = '\0';
	svm->PushString(&ret[0]);

	return 1;
}

static int sisdigit(SVM* svm, int numParams){
	checkParamsNum("string.isdigit", numParams);
	Value v = svm->PopStack();
	checkString("string.isdigit", v);

	string str = v.GetString();
	char c = str[0];
	if (c >= '0' && c <= '9'){
		svm->PushBool(true);
	}
	else{
		svm->PushBool(false);
	}

	return 1;
}

static int sisletter(SVM* svm, int numParams){
	checkParamsNum("string.isletter", numParams);
	Value v = svm->PopStack();
	checkString("string.isletter", v);

	string str = v.GetString();
	char c = str[0];
	if ((c >= 'a' && c <= 'z') ||
		(c >= 'A' && c <= 'Z')){
		svm->PushBool(true);
	}
	else{
		svm->PushBool(false);
	}

	return 1;
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

	return 1;
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
	
	svm->PushLightUData(f);

	return 1;
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

	return 1;
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
		Integer i = data.GetInteger();
		fwrite(&i, sizeof(Integer), 1, f);
	}
	else if (data.IsFloat()){
		Float fl = data.GetFloat();
		fwrite(&fl, sizeof(Float), 1, f);
	}
	else if (data.IsString()){
		string s = data.GetString(); 
		fwrite(s.c_str(), s.length()*sizeof(char), 1, f);
	}

	return 0;
}

static int close(SVM* svm, int numParams){
	Value file;
	checkParamsNum("io.close", numParams);
	file = svm->PopStack();
	checkLightUData("io.close", file);

	FILE* f = reinterpret_cast<FILE*>(file.GetLightUData());
	fclose(f);

	return 0;
}

static int allfile(SVM* svm, int numParams){
	Value directory, extension;
	checkParamsNum("io.allfile", numParams, 2);
	extension = svm->PopStack();
	directory = svm->PopStack();
	checkString("io.allfile", directory);
	checkString("io.allfile", extension);

	Tptr t = Tptr(new Table());
	string dir = directory.GetString() + "/*." + extension.GetString();
	int idx = 0;
	long handle;
	_finddata_t fi;
	if ((handle = _findfirst(dir.c_str(), &fi)) != -1L){
		if (fi.attrib != _A_SUBDIR && (strcmp(fi.name, ".") != 0 && strcmp(fi.name, "..") != 0))
			t->AddString(to_string(idx++), fi.name);
		while (_findnext(handle, &fi) == 0){
			if (fi.attrib == _A_SUBDIR) continue;
			if (strcmp(fi.name, ".") == 0 || strcmp(fi.name, "..") == 0)
				continue;
			t->AddString(to_string(idx++), fi.name);
		}
		_findclose(handle);

		svm->PushTable(t);
	}
	else
	{
		svm->PushInt(-1);
	}

	return 1;
}

static int alldirectory(SVM* svm, int numParams){
	Value directory;
	checkParamsNum("io.alldirectory", numParams);
	directory = svm->PopStack();
	checkString("io.alldirectory", directory);

	Tptr t = Tptr(new Table());
	string dir = directory.GetString() + "/*.*";
	int idx = 0;
	long handle;
	_finddata_t fi;
	if ((handle = _findfirst(dir.c_str(), &fi)) != -1L){
		if (fi.attrib == _A_SUBDIR && (strcmp(fi.name, ".") != 0 && strcmp(fi.name, "..") != 0))
			t->AddString(to_string(idx++), fi.name);
		while (_findnext(handle, &fi) == 0){
			if (fi.attrib != _A_SUBDIR) continue;
			if (strcmp(fi.name, ".") == 0 || strcmp(fi.name, "..") == 0)
				continue;
			t->AddString(to_string(idx++), fi.name);
		}
		_findclose(handle);

		svm->PushTable(t);
	}
	else
	{
		svm->PushInt(-1);
	}

	return 1;
}

static int input(SVM* svm, int numParams){
	checkParamsNum("io.gets", numParams, 0);

	char buf[1024] = { 0 };
	gets(buf);
	svm->PushString(buf);

	return 1;
}

//-------------------------------table lib----------------------
static int remove(SVM* svm, int numParams){
	checkParamsNum("table.remove", numParams, 2);
	Value s = svm->PopStack();
	Value t = svm->PopStack();
	checkTable("table.remove", t);
	checkString("table.remove", s, 2);

	Tptr table = t.GetTable();
	table->Remove(s.GetString());

	return 0;
}

static int tlength(SVM* svm, int numParams){
	checkParamsNum("table.len", numParams);
	Value t = svm->PopStack();
	checkTable("table.len", t);

	Tptr table = t.GetTable();
	svm->PushInt(table->GetLength());

	return 1;
}

static int tforeach(SVM* svm, int numParams){
	checkParamsNum("table.foreach", numParams, 2);
	Value table, func;
	func = svm->PopStack();
	table = svm->PopStack();
	checkTable("table.foreach", table);
	checkFunction("table.foreach", func, 2);

	Tptr t = table.GetTable();
	TableIteration ti = t->Begin();
	TableIteration tied = t->End();
	for (; ti != tied; ++ti){
		Value key, value;
		key.SetString(ti->first);
		value = ti->second;

		svm->PushStack(key);
		svm->PushStack(value);
		svm->PushStack(func);
		svm->CallScript(2);
	}

	return 0;
}

static int tcopy(SVM* svm, int numParams){
	checkParamsNum("table.copy", numParams, 2);
	Value src = svm->PopStack();
	Value dest = svm->PopStack();
	checkTable("table.copy", dest);
	checkTable("table.copy", src, 2);

	Tptr s = src.GetTable();
	Tptr d = dest.GetTable();
	*d = *s;

	return 0;
}

//------------------------------coroutine lib-------------------
struct CallInfo{
	int cp;
	int ap;
	int fp;
	int of;
};
static hash_map<Coptr, vector<CallInfo> > ccmap;

void cocallback(SVM* svm){
	Coptr co = svm->PopCo();
	co->status = ECoroutineStatus::EDEAD;
	ccmap.erase(ccmap.find(co));
}

static int cocreate(SVM* svm, int numParams){
	checkParamsNum("coroutine.create", numParams);
	Value func = svm->PopStack();
	checkFunction("coroutine.create", func);

	int p = func.GetFunction()->entry;
	Coptr co = Coptr(new Coroutine());
	co->cl = func.GetFunction();
	co->ip = p;
	co->status = ECoroutineStatus::ESTART;
	svm->PushCoroutine(co);

	return 1;
}

static int coresume(SVM* svm, int numParams){
	checkParamsNumg("coroutine.resume", numParams);
	vector<Value> p(numParams);
	for (int i = 0; i < numParams; ++i)
		p[numParams - i - 1] = svm->PopStack();

	checkCoroutine("coroutine.resume", p[0]);

	Coptr co = p[0].GetCoroutine();
	svm->PushCo(co);

	ECoroutineStatus status = co->status;
	co->status = ECoroutineStatus::ERUNING;

	SVM::Register r = svm->GetRegister();
	if (status == ECoroutineStatus::EDEAD){
		//当协程死亡时返回false
		svm->PopCo();
		svm->PushBool(false);
		co->status = ECoroutineStatus::EDEAD;
		return 1;
	}
	else if (status == ECoroutineStatus::ESTART){
		Value func;
		func.SetFunction(co->cl);
		co->ip = r.ip; 
		int ap = numParams - 1;
		int cp = r.sp;
		int fp = co->cl->fp;
		int offset = ap - fp;
		ccmap[co].push_back({ cp, ap, fp, offset });
		ap = r.ap;
		cp = r.cp;
		fp = r.fp;
		offset = ap - fp;
		ccmap[co].push_back({ cp, ap, fp, offset });

		for (int i = 0; i < numParams - 1; ++i)
			svm->PushStack(p[i + 1]);
		svm->PushStack(func);
		svm->CallScript(numParams - 1);
	}
	else if (status == ECoroutineStatus::ESUSPENDED){
		checkParamsNuml("coroutine.resume", numParams, 2);

		CallInfo ci = ccmap[co][ccmap[co].size() - 1];
		ccmap[co].pop_back();
		ccmap[co].push_back({ r.cp, r.ap, r.fp, r.of });
		SVM::Register restore;
		restore.ip = co->ip;
		restore.sp = r.sp;
		restore.cp = ci.cp;
		restore.of = ci.of;
		restore.fp = ci.fp;
		restore.ap = ci.ap;
		svm->SetRegister(restore);
		if (numParams == 2) svm->PushStack(p[1]);

		Value vip;
		vip.SetInt((r.ip + 1) | (1 << 31));//协程标志
		if (ccmap[co][0].cp == ci.cp)
			svm->SetStack(ci.cp + ci.ap, vip);
		co->ip = r.ip;
	}

	return 0;
}

static int coyield(SVM* svm, int numParams){
	checkParamsNuml("coroutine.yield", numParams);
	
	Coptr co = svm->PopCo();
	if (!co) return 0;
	co->status = ECoroutineStatus::ESUSPENDED;
	SVM::Register r = svm->GetRegister();
	CallInfo ci;
	ci = ccmap[co][ccmap[co].size() - 1];
	ccmap[co].pop_back();
	int ip = r.ip;
	int cp = r.cp;
	int offset = r.of;
	int ap = r.ap;
	int fp = r.fp;
	r.ip = co->ip;
	r.cp = ci.cp;
	r.of = ci.of;
	r.fp = ci.fp;
	r.ap = ci.ap;
	svm->SetRegister(r);
	
	co->ip = ip;
	ccmap[co].push_back({ cp, ap, fp, offset });

	return 0;
}

static int costatus(SVM* svm, int numParams){
	checkParamsNum("coroutine.status", numParams);
	Value v = svm->PopStack();
	checkCoroutine("coroutine.status", v);

	string ret;
	Coptr co = v.GetCoroutine();
	ECoroutineStatus status = co->status;
	switch(status){
	case ECoroutineStatus::ESTART:
		ret = "start";
		break;
	case ECoroutineStatus::ERUNING:
		ret = "runing";
		break;
	case ECoroutineStatus::ESUSPENDED:
		ret = "suspended";
		break;
	case ECoroutineStatus::EDEAD:
		ret = "dead";
		break;
	}

	svm->PushString(ret);

	return 1;
}

//------------------------------func register-------------------
void registerConstant(shared_ptr<Environment>& e, shared_ptr<SVM>& svm){
	Value table;
	Tptr t = Tptr(new Table());
	table.SetTable(t);
	int idx = svm->AddGlobal(table);
	SymbolInfo si = { table, idx };
	e->SetSymbol("constant", si);
	t->AddInt("INT_MAX", SABER_MAX_INTEGER);
	t->AddInt("INT_MIN", SABER_MIN_INTEGER);
	t->AddFloat("INFINITY", SABER_INFINITY);
	t->AddFloat("PI", PI);
	t->AddFloat("E", E);
}

static RegisterFunction basic[] = {
	{ "print", print },
	{ "type", type },
	{ "seed", seed },
	{ "uniformInt", uniformInt },
	{ "uniformFloat", uniformFloat },
	{ "load", load },
	{ "next", next },
	{ "reverseBit", reverseBit },
	{ "toint", toint },
	{ "tostring", tostring },
	{ "isnull", isnull },
	{ "isbool", isbool },
	{ "isint", isint },
	{ "isfloat", isfloat },
	{ "isnumber", isnumber },
	{ "isstring", isstring },
	{ "isfunction", isfunction },
	{ "isnativefunction", isnativefunction },
	{ "istable", istable },
	{ "iscoroutine", iscoroutine },
	{ "ispointer", ispointer },
	{ "vfp", getvfrompointer },
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
	{ "log2", log2 },
	{ "log10", log10 },
	{ "exp", exp },
	{ "sqrt", sqrt },
	{ "pow", pow },
	{ "floor", floor },
	{ "ceil", ceil },
	{ "max", mmax },
	{ "min", mmin },
	{ "", nullptr },
};

void registerMath(shared_ptr<Environment>& e, shared_ptr<SVM>& svm){
	Value table;
	Tptr t = Tptr(new Table());
	table.SetTable(t);
	int idx = svm->AddGlobal(table);
	SymbolInfo si = { table, idx };
	e->SetSymbol("math", si);
	for (int i = 0;; ++i){
		string name = math[i].name;
		if (name == "") break;

		t->AddNativeFunction(name, math[i].f);
	}
}

static RegisterFunction os[] = {
	{ "gettime", gettime },
	{ "time", gettimei },
	{ "getclock", getclock },
	{ "exit", osexit },
	{ "sleep", osleep },
	{ "", nullptr },
};

void registerOs(shared_ptr<Environment>& e, shared_ptr<SVM>& svm){
	Value table;
	Tptr t = Tptr(new Table());
	table.SetTable(t);
	int idx = svm->AddGlobal(table);
	SymbolInfo si = { table, idx };
	e->SetSymbol("os", si);
	for (int i = 0;; ++i){
		string name = os[i].name;
		if (name == "") break;

		t->AddNativeFunction(name, os[i].f);
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
	{ "at", at },
	{ "foreach", sforeach },
	{ "format", format },
	{ "lower", lower },
	{ "upper", upper },
	{ "isdigit", sisdigit },
	{ "isletter", sisletter },
	{ "", nullptr },
};

void registerStr(shared_ptr<Environment>& e, shared_ptr<SVM>& svm){
	Value table;
	Tptr t = Tptr(new Table());
	table.SetTable(t);
	int idx = svm->AddGlobal(table);
	SymbolInfo si = { table, idx };
	e->SetSymbol("string", si);
	for (int i = 0;; ++i){
		string name = str[i].name;
		if (name == "") break;

		t->AddNativeFunction(name, str[i].f);
	}
}

static RegisterFunction io[] = {
	{ "exist", fexist },
	{ "open", open },
	{ "read", read },
	{ "write", write },
	{ "close", close },
	{ "allfile", allfile },
	{ "alldirectory", alldirectory },
	{ "input", input },
	{ "", nullptr },
};

void registerIo(shared_ptr<Environment>& e, shared_ptr<SVM>& svm){
	Value table;
	Tptr t = Tptr(new Table());
	table.SetTable(t);
	int idx = svm->AddGlobal(table);
	SymbolInfo si = { table, idx };
	e->SetSymbol("io", si);
	for (int i = 0;; ++i){
		string name = io[i].name;
		if (name == "") break;

		t->AddNativeFunction(name, io[i].f);
	}
}

static RegisterFunction tb[] = {
	{ "remove", remove },
	{ "len", tlength },
	{ "foreach", tforeach },
	{ "copy", tcopy },
	{ "", nullptr },
};

void registerTable(shared_ptr<Environment>& e, shared_ptr<SVM>& svm){
	Value table;
	Tptr t = Tptr(new Table());
	table.SetTable(t);
	int idx = svm->AddGlobal(table);
	SymbolInfo si = { table, idx };
	e->SetSymbol("table", si);
	for (int i = 0;; ++i){
		string name = tb[i].name;
		if (name == "") break;

		t->AddNativeFunction(name, tb[i].f);
	}
}

static RegisterFunction co[] = {
	{ "create", cocreate },
	{ "resume", coresume },
	{ "yield", coyield },
	{ "status", costatus },
	{ "", nullptr },
};

void registerCoroutine(shared_ptr<Environment>& e, shared_ptr<SVM>& svm){
	Value table;
	Tptr t = Tptr(new Table());
	table.SetTable(t);
	int idx = svm->AddGlobal(table);
	SymbolInfo si = { table, idx };
	e->SetSymbol("coroutine", si);
	for (int i = 0;; ++i){
		string name = co[i].name;
		if (name == "") break;

		t->AddNativeFunction(name, co[i].f);
	}
}

void NativeFunc::Register(shared_ptr<Environment>& e, shared_ptr<SVM>& svm){
	registerConstant(e, svm);
	registerBasic(e, svm);
	registerMath(e, svm);
	registerOs(e, svm);
	registerStr(e, svm);
	registerIo(e, svm);
	registerTable(e, svm);
	registerCoroutine(e, svm);
}

SABER_NAMESPACE_END