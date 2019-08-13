#ifndef H_VM_H
#define H_VM_H

#include "common.h"
#include "value.h"

class VM{
protected:
#define NUM_REGISTER  64
	vector<int> code;
	vector<Value> stack;
	vector<Value> constant;
	vector<Value> registers;

	int pc;
	int fp;
	int sp;
	int ret;

public:
	VM();

	int AddCode(int c);
	void SetCode(int idx, int c);
	int AddConstant(Value v);
	int AddStack(Value v);
	int AddRegister(Value v);

	void Run();

	string ShowCode();

private:
	void move();
	void jz();
	void jump();
	void call();
	void calln();
	void add();
	void sub();
	void mul();
	void div();
	void mod();
	void gt();
	void gte();
	void equal();
	void nequal();

	bool isStack(int idx);
	bool isRegister(int idx);
	int encodeRegisterIndex(int idx);
	int decodeRegisterIndex(int idx);
	int encodeConstantIndex(int idx);
	int decodeConstantIndex(int idx);
};

#endif