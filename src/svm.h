#ifndef H_SVM_H
#define H_SVM_H

#include "common.h"
#include "value.h"

class SVM{
public:
	struct Instruction{
		char opcode;
		int operand;
		int operand1;
	};

protected:
#define NUM_REGISTER  64
	vector<Instruction> code;
	vector<Value> stack;
	vector<Value> constant;
	vector<Value> global;
	vector<Value> registers;

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

#endif