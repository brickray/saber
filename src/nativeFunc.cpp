#include "nativeFunc.h"
#include "astree.h"

#include <time.h>
#include <Windows.h>

SABER_NAMESPACE_BEGIN

void mathParamsCheck(string func, Value& v, int numParams){
	bool pass = true;
	if (numParams > 1){
		printf("%s函数只接收一个参数\n", func.c_str());
		pass = false;
	}
	if (!(v.IsFloat() || v.IsInteger())){
		printf("%s函数参数类型必须为整数或浮点数\n", func.c_str());
		pass = false;
	}

	if (!pass) exit(1);
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

static int sin(SVM* svm, int numParams){
	Value v = svm->PopStack();
	mathParamsCheck("sin", v, numParams);

	float ret;
	if (v.IsFloat()) ret = sinf(DegreeToRadians(v.GetFloat()));
	else if (v.IsInteger()) ret = sinf(DegreeToRadians(v.GetInteger()));

	svm->PushFloat(ret);

	return numParams;
}

static int asin(SVM* svm, int numParams){
	Value v = svm->PopStack();
	mathParamsCheck("asin", v, numParams);

	float ret;
	if (v.IsFloat()) ret = asinf(v.GetFloat());
	else if (v.IsInteger()) ret = asinf(v.GetInteger());

	svm->PushFloat(ret);

	return numParams;
}

static int cos(SVM* svm, int numParams){
	Value v = svm->PopStack();
	mathParamsCheck("cos", v, numParams);

	float ret;
	if (v.IsFloat()) ret = cosf(DegreeToRadians(v.GetFloat()));
	else if (v.IsInteger()) ret = cosf(DegreeToRadians(v.GetInteger()));

	svm->PushFloat(ret);

	return numParams;
}

static int acos(SVM* svm, int numParams){
	Value v = svm->PopStack();
	mathParamsCheck("acos", v, numParams);

	float ret;
	if (v.IsFloat()) ret = acosf(v.GetFloat());
	else if (v.IsInteger()) ret = acosf(v.GetInteger());

	svm->PushFloat(ret);

	return numParams;
}

static int tan(SVM* svm, int numParams){
	Value v = svm->PopStack();
	mathParamsCheck("tan", v, numParams);

	float ret;
	if (v.IsFloat()) ret = tanf(DegreeToRadians(v.GetFloat()));
	else if (v.IsInteger()) ret = tanf(DegreeToRadians(v.GetInteger()));

	svm->PushFloat(ret);

	return numParams;
}

static int atan(SVM* svm, int numParams){
	Value v = svm->PopStack();
	mathParamsCheck("atan", v, numParams);

	float ret;
	if (v.IsFloat()) ret = atanf(v.GetFloat());
	else if (v.IsInteger()) ret = atanf(v.GetInteger());

	svm->PushFloat(ret);

	return numParams;
}

static int radians(SVM* svm, int numParams){
	Value v = svm->PopStack();
	mathParamsCheck("radians", v, numParams);

	float ret;
	if (v.IsFloat()) ret = DegreeToRadians(v.GetFloat());
	else if (v.IsInteger()) ret = DegreeToRadians(v.GetInteger());

	svm->PushFloat(ret);

	return numParams;
}

static int degree(SVM* svm, int numParams){
	Value v = svm->PopStack();
	mathParamsCheck("degree", v, numParams);

	float ret;
	if (v.IsFloat()) ret = RadiansToDegree(v.GetFloat());
	else if (v.IsInteger()) ret = RadiansToDegree(v.GetInteger());

	svm->PushFloat(ret);

	return numParams;
}

static int gettime(SVM* svm, int numParams){
	if (numParams > 0){
		printf("gettime函数参数过多\n");
		exit(1);
	}
	time_t t = time(nullptr);
	string str = ctime(&t);
	//去掉末尾的换行符
	str = str.substr(0, str.size() - 1);
	svm->PushString(str);

	return numParams;
}

static int getclock(SVM* svm, int numParams){
	if (numParams > 0){
		printf("gettime函数参数过多\n");
		exit(1);
	}
	
	LARGE_INTEGER start, freq;
	QueryPerformanceCounter(&start);
	QueryPerformanceFrequency(&freq);
	float r = float(start.QuadPart) / freq.QuadPart;

	svm->PushFloat(r);

	return numParams;
}

static RegisterFunction native[] = {
	{ "print", print },
	{ "sin", sin },
	{ "asin", asin },
	{ "cos", cos },
	{ "acos", acos },
	{ "tan", tan },
	{ "atan", atan },
	{ "radians", radians },
	{ "degree", degree },
	{ "gettime", gettime },
	{ "getclock", getclock },
	{ "", nullptr },
};

void NativeFunc::Register(shared_ptr<Environment>& e, shared_ptr<SVM>& svm){
	for (int i = 0;; ++i){
		string name = native[i].name;
		if (name == "") break;

		SValue sv;
		sv.sfunc = native[i].f;
		Value function(EValueType::ENATIVEFUNC, sv);
		int idx = svm->AddGlobal(function);
		SymbolInfo si = { function, idx };
		e->SetSymbol(name, si);
	}
}

SABER_NAMESPACE_END