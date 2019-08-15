#include "nativeFunc.h"
#include "astree.h"

#include <time.h>

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
	if (v.IsFloat()) ret = sin(DegreeToRadians(v.GetFloat()));
	else if (v.IsInteger()) ret = sin(DegreeToRadians(v.GetInteger()));

	svm->PushFloat(ret);

	return numParams;
}

static int asin(SVM* svm, int numParams){
	Value v = svm->PopStack();
	mathParamsCheck("asin", v, numParams);

	float ret;
	if (v.IsFloat()) ret = asin(v.GetFloat());
	else if (v.IsInteger()) ret = asin(v.GetInteger());

	svm->PushFloat(ret);

	return numParams;
}

static int cos(SVM* svm, int numParams){
	Value v = svm->PopStack();
	mathParamsCheck("cos", v, numParams);

	float ret;
	if (v.IsFloat()) ret = cos(DegreeToRadians(v.GetFloat()));
	else if (v.IsInteger()) ret = cos(DegreeToRadians(v.GetInteger()));

	svm->PushFloat(ret);

	return numParams;
}

static int acos(SVM* svm, int numParams){
	Value v = svm->PopStack();
	mathParamsCheck("acos", v, numParams);

	float ret;
	if (v.IsFloat()) ret = acos(v.GetFloat());
	else if (v.IsInteger()) ret = acos(v.GetInteger());

	svm->PushFloat(ret);

	return numParams;
}

static int tan(SVM* svm, int numParams){
	Value v = svm->PopStack();
	mathParamsCheck("tan", v, numParams);

	float ret;
	if (v.IsFloat()) ret = tan(DegreeToRadians(v.GetFloat()));
	else if (v.IsInteger()) ret = tan(DegreeToRadians(v.GetInteger()));

	svm->PushFloat(ret);

	return numParams;
}

static int atan(SVM* svm, int numParams){
	Value v = svm->PopStack();
	mathParamsCheck("atan", v, numParams);

	float ret;
	if (v.IsFloat()) ret = atan(v.GetFloat());
	else if (v.IsInteger()) ret = atan(v.GetInteger());

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
	svm->PushString(ctime(&t));

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