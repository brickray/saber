#ifndef H_SVM_H
#define H_SVM_H

#include "common.h"
#include "value.h"

SABER_NAMESPACE_BEGIN

#define STACK_SIZE   4096
#define IP_ADDRESS   0
#define SP_ADDRESS   1
#define CP_ADDRESS   2
#define OF_ADDRESS   3
#define AP_ADDRESS   4
#define FP_ADDRESS   5 
#define CL_ADDRESS   6
#define TB_ADDRESS   7
#define NUM_ADDRESS  8

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
		int of;
		int ap;
		int fp;
	};

protected:
	vector<Instruction> code;
	vector<Value> stack;
	vector<Value> global;
	vector<Coptr> co; //协程栈

	/*call stack
	 |  param  |    parameters
	 |    ip   |
	 |    sp   |
	 |    cp   |
	 |    of   |
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
	int of;
	int ap;
	int fp;
	Clptr cl;
	int codeSize; //代码长度

	SState* S;

public:
	SVM(SState* s);

	int AddCode(Instruction& c);
	void RemoveLastCode();
	Instruction GetLastCode();
	Instruction GetCode(int idx);
	void SetCode(int idx, Instruction& c);
	int AddGlobal(Value& v);
	void SetStack(int i, Value& v);
	void PushStack(Value& v);
	void PushBool(bool b);
	void PushInt(int i);
	void PushFloat(float f);
	void PushString(string s);
	void PushFunc(Clptr cl);
	void PushNativeFunc(SFunc f);
	void PushLightUData(int i);
	void PushTable(Tptr t);
	void PushCoroutine(Coptr co); //压入到全局栈
	Value PopStack();

	void Run();
	void CallScript(int numParams);

	SState* GetSState() const { return S; }
	Register GetRegister() const { return{ ip, sp, cp, of, ap, fp }; }
	void SetRegister(Register r) { ip = r.ip; sp = r.sp; cp = r.cp; of = r.of; ap = r.ap; fp = r.fp; }
	void UpdateCodeSize() { codeSize = code.size(); }
	//压入到辅助结构
	void PushCo(Coptr c); 
	Coptr PopCo(); 
	int GetCoSize() const { return co.size(); }
	string ShowCode();

private:
	void execute();
	bool isStack(int idx);
	bool isGlobal(int idx);
	int encodeGlobalIndex(int idx);
	int decodeGlobalIndex(int idx);

	void constructTDot(Tptr t, int fp, int ap);
	Clptr createClosure(Clptr o);
	void overrideOp(Value& t, const char* opname, int np, const char* op);
	Value* getAddress(Instruction& ins);
	void move(Instruction& ins);

	void dumpStack();
};

SABER_NAMESPACE_END

#endif