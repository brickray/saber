#ifndef H_SVM_H
#define H_SVM_H

#include "common.h"
#include "value.h"

SABER_NAMESPACE_BEGIN

class SVM{
public:
	struct Instruction{
		char opcode;
		int operand;
	};

protected:
	vector<Instruction> code;
	vector<Value> stack;
	vector<Value> constant;
	vector<Value> global;

	int ip;
	int sp;
	int fp;
	int ret;

public:
	SVM();

	int AddCode(Instruction c);
	void SetCode(int idx, Instruction c);
	int AddGlobal(Value v);
	int AddConstant(Value v);
	void PushStack(Value v);
	void PushBool(bool b);
	void PushInt(int i);
	void PushFloat(float f);
	void PushString(string s);
	Value PopStack();

	void Run();

	string ShowCode();

private:
	bool isGlobal(int idx);
	int encodeConstantIndex(int idx);
	int decodeConstantIndex(int idx);
};

SABER_NAMESPACE_END

#endif