#include "nativeFunc.h"
#include "astree.h"
#include "error.h"

#include <time.h>

SABER_NAMESPACE_BEGIN

void mathParamsNumCheck(string func, int numParams){
	if (numParams != 1){
		Error::GetInstance()->ProcessError("%s函数只接收一个参数\n", func.c_str());
	}

	return;
}

void mathParamsTypeCheck(string func, Value& v){
	if (!(v.IsFloat() || v.IsInteger())){
		Error::GetInstance()->ProcessError("%s函数参数类型必须为整数或浮点数\n", func.c_str());
	}

	return;
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
	mathParamsNumCheck("sin", numParams);
	Value v = svm->PopStack();
	mathParamsTypeCheck("sin", v);

	float ret;
	if (v.IsFloat()) ret = sinf(DegreeToRadians(v.GetFloat()));
	else if (v.IsInteger()) ret = sinf(DegreeToRadians(v.GetInteger()));

	svm->PushFloat(ret);

	return numParams;
}

static int asin(SVM* svm, int numParams){
	mathParamsNumCheck("asin", numParams);
	Value v = svm->PopStack();
	mathParamsTypeCheck("asin", v);

	float ret;
	if (v.IsFloat()) ret = asinf(v.GetFloat());
	else if (v.IsInteger()) ret = asinf(v.GetInteger());

	svm->PushFloat(ret);

	return numParams;
}

static int cos(SVM* svm, int numParams){
	mathParamsNumCheck("cos", numParams);
	Value v = svm->PopStack();
	mathParamsTypeCheck("cos", v);

	float ret;
	if (v.IsFloat()) ret = cosf(DegreeToRadians(v.GetFloat()));
	else if (v.IsInteger()) ret = cosf(DegreeToRadians(v.GetInteger()));

	svm->PushFloat(ret);

	return numParams;
}

static int acos(SVM* svm, int numParams){
	mathParamsNumCheck("acos", numParams);
	Value v = svm->PopStack();
	mathParamsTypeCheck("acos", v);

	float ret;
	if (v.IsFloat()) ret = acosf(v.GetFloat());
	else if (v.IsInteger()) ret = acosf(v.GetInteger());

	svm->PushFloat(ret);

	return numParams;
}

static int tan(SVM* svm, int numParams){
	mathParamsNumCheck("tan", numParams);
	Value v = svm->PopStack();
	mathParamsTypeCheck("tan", v);

	float ret;
	if (v.IsFloat()) ret = tanf(DegreeToRadians(v.GetFloat()));
	else if (v.IsInteger()) ret = tanf(DegreeToRadians(v.GetInteger()));

	svm->PushFloat(ret);

	return numParams;
}

static int atan(SVM* svm, int numParams){
	mathParamsNumCheck("atan", numParams);
	Value v = svm->PopStack();
	mathParamsTypeCheck("atan", v);

	float ret;
	if (v.IsFloat()) ret = atanf(v.GetFloat());
	else if (v.IsInteger()) ret = atanf(v.GetInteger());

	svm->PushFloat(ret);

	return numParams;
}

static int radians(SVM* svm, int numParams){
	mathParamsNumCheck("radians", numParams);
	Value v = svm->PopStack();
	mathParamsTypeCheck("radians", v);

	float ret;
	if (v.IsFloat()) ret = DegreeToRadians(v.GetFloat());
	else if (v.IsInteger()) ret = DegreeToRadians(v.GetInteger());

	svm->PushFloat(ret);

	return numParams;
}

static int degree(SVM* svm, int numParams){
	mathParamsNumCheck("degree", numParams);
	Value v = svm->PopStack();
	mathParamsTypeCheck("degree", v);

	float ret;
	if (v.IsFloat()) ret = RadiansToDegree(v.GetFloat());
	else if (v.IsInteger()) ret = RadiansToDegree(v.GetInteger());

	svm->PushFloat(ret);

	return numParams;
}

static int gettime(SVM* svm, int numParams){
	if (numParams > 0){
		Error::GetInstance()->ProcessError("gettime函数参数过多\n");
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
		Error::GetInstance()->ProcessError("getclock函数参数过多\n");
	}
	
	clock_t t = clock();
	t /= CLOCKS_PER_SEC;

	svm->PushInt(t);

	return numParams;
}

static int type(SVM* svm, int numParams){
	if (numParams != 1){
		Error::GetInstance()->ProcessError("type函数只接受1个参数\n");
	}

	Value v = svm->PopStack();
	
	svm->PushString(v.GetTypeString());

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
	{ "type", type },
	{ "", nullptr },
};

void NativeFunc::Register(shared_ptr<Environment>& e, shared_ptr<SVM>& svm){
	for (int i = 0;; ++i){
		string name = native[i].name;
		if (name == "") break;

		Value function;
		function.SetNativeFunction(native[i].f);
		int idx = svm->AddGlobal(function);
		SymbolInfo si = { function, idx };
		e->SetSymbol(name, si);
	}
}

SABER_NAMESPACE_END