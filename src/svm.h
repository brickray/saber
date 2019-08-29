#ifndef H_SVM_H
#define H_SVM_H

#include "common.h"
#include "value.h"

SABER_NAMESPACE_BEGIN

class SState;
class SVM{
public:
	struct Instruction{
		char opcode;
		bool relative;
		char padding0;
		char padding1;
		union{
			int operand;
			float operandf;
		};

		Instruction(char opc, int ope = 0, bool r = false)
		:opcode(opc), operand(ope), relative(r){}


		Instruction(char opc, float ope)
			:opcode(opc), operandf(ope), relative(false){}
	};

protected:
#define STACK_SIZE 1024
	vector<Instruction> code;
	vector<Value> stack;
	vector<Value> constant;
	vector<Value> global;
	struct CallInfo{
		int ap;
		int fp;
		int offset;
	};
	vector<CallInfo> nps;

	int ip; //���������
	int sp;	//ջ��ָ��
	int cp; //����ջָ��
	int offset;

	SState* S;

public:
	SVM(SState* s);

	int AddCode(Instruction c);
	void RemoveLastCode();
	Instruction GetLastCode();
	Instruction GetCode(int idx);
	void SetCode(int idx, Instruction c);
	int AddGlobal(Value v);
	int AddConstant(Value v);
	void PushStack(Value v);
	void PushBool(bool b);
	void PushInt(int i);
	void PushFloat(float f);
	void PushString(string s);
	void PushFunc(int i);
	void PushNativeFunc(SFunc f);
	void PushLightUData(int i);
	void PushTable(int i);
	Value PopStack();

	void Run();
	void CallScript(int numParams);

	SState* GetSState() const { return S; }
	string ShowCode();

private:
	void execute();
	bool isStack(int idx);
	bool isGlobal(int idx);
	int encodeGlobalIndex(int idx);
	int decodeGlobalIndex(int idx);
	int encodeConstantIndex(int idx);
	int decodeConstantIndex(int idx);

	void constructTDot(Table* t, int fp, int ap);
};

SABER_NAMESPACE_END

#endif