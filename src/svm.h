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
#define STACK_SIZE 1024
	vector<Instruction> code;
	vector<Value> stack;
	vector<Value> constant;
	vector<Value> global;

	int ip; //���������
	int sp;	//ջ��ָ��
	int cp; //

public:
	SVM();

	int AddCode(Instruction c);
	void RemoveLastCode();
	Instruction GetCode(int idx);
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
	bool isStack(int idx);
	bool isGlobal(int idx);
	int encodeGlobalIndex(int idx);
	int decodeGlobalIndex(int idx);
	int encodeConstantIndex(int idx);
	int decodeConstantIndex(int idx);
};

SABER_NAMESPACE_END

#endif