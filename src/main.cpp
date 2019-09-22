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

static int setx(SVM* svm, int numParams){
	checkParamsNum("CTest.setx", numParams);
	Value v = svm->PopStack();
	Value x = svm->PopStack();
	checkTable("CTest.setx", v);
	checkInteger("CTest.setx", x, 2);

	CTest* t = reinterpret_cast<CTest*>(v.GetTable()->GetValue(SELF).GetLightUData());
	t->SetX(x.GetInteger());

	return 0;
}

static int sety(SVM* svm, int numParams){
	checkParamsNum("CTest.sety", numParams);
	Value v = svm->PopStack();
	Value x = svm->PopStack();
	checkTable("CTest.sety", v);
	checkInteger("CTest.sety", x, 2);

	CTest* t = reinterpret_cast<CTest*>(v.GetTable()->GetValue(SELF).GetLightUData());
	t->SetY(x.GetInteger());

	return 0;
}

static int add(SVM* svm, int numParams){
	checkParamsNum("CTest,add", numParams, 0);
	Value v = svm->PopStack();
	checkTable("CTest.add", v);

	CTest* t = reinterpret_cast<CTest*>(v.GetTable()->GetValue(SELF).GetLightUData());
	svm->PushInt(t->Add());

	return 1;
}

static int create(SVM* svm, int numParams){
	checkParamsNum("CTest.create", numParams, 0);
	CTest* t = new CTest();
	Tptr table = Tptr(new Table());
	table->AddNativeFunction("setx", setx);
	table->AddNativeFunction("sety", sety);
	table->AddNativeFunction("add", add);
	table->AddLightUData(SELF, t);
	
	svm->PushTable(table);

	return 1;
}

RegisterFunction test[] = {
	{ "create", create },
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

void main(int argc, char** argv){
	ifstream stream;
	stream.open(argv[1]);
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

//	CppCallSaber();
}