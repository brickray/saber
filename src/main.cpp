#include <stdio.h>
#include <fstream>
#include <sstream>
#include <time.h>
#include "saber.h"
#include "sstate.h"

SABER_NAMESPACE_USING

SState state;

class CTest{
public:
	int x, y;

public:
	void SetX(int i){
		x = i;
	}

	void SetY(int i){
		y = i;
	}

	int Add(){
		return x + y;
	}
};

static int create(SVM* svm, int numParams){
	checkParamsNum("test.create", numParams, 0);
	CTest* t = new CTest();
	
	svm->PushLightUData(int(t));

	return 1;
}

static int setx(SVM* svm, int numParams){
	checkParamsNum("test.setx", numParams, 2);
	Value x = svm->PopStack();
	Value v = svm->PopStack();
	checkLightUData("test.setx", v);
	checkInteger("test.setx", x, 2);

	CTest* t = reinterpret_cast<CTest*>(v.GetLightUData());
	t->SetX(x.GetInteger());

	return 0;
}

static int sety(SVM* svm, int numParams){
	checkParamsNum("test.sety", numParams, 2);
	Value x = svm->PopStack();
	Value v = svm->PopStack();
	checkLightUData("test.sety", v);
	checkInteger("test.sety", x, 2);

	CTest* t = reinterpret_cast<CTest*>(v.GetLightUData());
	t->SetY(x.GetInteger());

	return 0;
}

static int add(SVM* svm, int numParams){
	checkParamsNum("test,add", numParams, 1);
	Value v = svm->PopStack();
	checkLightUData("test.add", v);

	CTest* t = reinterpret_cast<CTest*>(v.GetLightUData());
	svm->PushInt(t->Add());

	return numParams;
}

RegisterFunction test[] = {
	{ "create", create },
	{ "setx", setx },
	{ "sety", sety },
	{ "add", add },
	{ "", nullptr },
};

void CppCallSaber(){
	state.PushInt(10);
	state.PushInt(2);
	state.PushFloat(0.5);
	for (int i = 0; i < 100; ++i){
		state.PushInt(5);
	}
	state.PushFunction("add");
	state.CallScript(103);
	Value ret = state.PopStack();
	printf("%s\n", ret.ToString().c_str());
}

void main(){
	ifstream stream;
	stream.open("../test/script.sa");
	if (!stream.is_open()){
		printf("unable to open file\n");
		return;
	}
	stringstream buffer;
	buffer << stream.rdbuf();
	string code(buffer.str());

	state.Init();
	state.RegisterLib("test", test);

	state.ShowCode(true);
	state.Compile(code);
	state.Run();

	CppCallSaber();

	getchar();
}