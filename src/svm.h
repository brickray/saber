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
		string operands;

		Instruction(char opc, int ope = 0, bool r = false)
		:opcode(opc), operand(ope), relative(r){}

		Instruction(char opc, float ope)
			:opcode(opc), operandf(ope), relative(false){}

		Instruction(char opc, string ope)
			:opcode(opc), operands(ope), relative(false){}

		Instruction(char opc, int ope1, string ope2)
			:opcode(opc), operand(ope1), operands(ope2), relative(false){}
	};

	struct Register{
		int ip;
		int sp;
		int cp;
		int offset;
		int ap;
		int fp;
	};

protected:
#define STACK_SIZE 1024
	vector<Instruction> code;
	vector<Value> stack;
	vector<Value> global;
	vector<Coroutine*> co; //协程栈

	/*call stack
	 |  param  |    parameters
	 |    ip   |
	 |    sp   |
	 |    cp   |
	 |  offset |
	 |    ap   |    number of actual param
	 |    fp   |    number of formal param
	 | closure |
	 |    tb   |
	 |  localp |    local variable
	 |         | -> stack top
	*/
	int ip; //代码计数器
	int sp;	//栈顶指针
	int cp; //函数栈指针
	int offset;
	int ap;
	int fp;
	Clptr cl;

	SState* S;

public:
	SVM(SState* s);

	int AddCode(Instruction c);
	void RemoveLastCode();
	Instruction GetLastCode();
	Instruction GetCode(int idx);
	void SetCode(int idx, Instruction c);
	int AddGlobal(Value v);
	void SetStack(int i, Value v);
	void PushStack(Value v);
	void PushBool(bool b);
	void PushInt(int i);
	void PushFloat(float f);
	void PushString(string s);
	void PushFunc(Clptr cl);
	void PushNativeFunc(SFunc f);
	void PushLightUData(int i);
	void PushTable(Tptr t);
	void PushCoroutine(Coroutine* co); //压入到全局栈
	Value PopStack();

	void Run();
	void CallScript(int numParams);

	SState* GetSState() const { return S; }
	Register GetRegister() const { return{ ip, sp, cp, offset, ap, fp }; }
	void SetRegister(Register r) { ip = r.ip; sp = r.sp; cp = r.cp; offset = r.offset; ap = r.ap; fp = r.fp; }
	bool IsEnd() const { return ip >= code.size(); }
	//压入到辅助结构
	void PushCo(Coroutine* c); 
	Coroutine* PopCo(); 
	int GetCoSize() const { return co.size(); }
	string ShowCode();

private:
	void execute();
	bool isStack(int idx);
	bool isGlobal(int idx);
	int encodeGlobalIndex(int idx);
	int decodeGlobalIndex(int idx);

	void constructTDot(Tptr t, int fp, int ap);
	Value getClosureValue(int op);
	void setClosureValue(int op, Value v);
	Clptr createClosure(Clptr o);
};

SABER_NAMESPACE_END

#endif