#ifndef H_AST_ASTCLOSURE_H
#define H_AST_ASTCLOSURE_H

#include "../astree.h"

SABER_NAMESPACE_BEGIN

class AstClosure : public Astree{
private:
	int numParams = 0;
	bool variable = false;
public:
	void SetNumParams(int n) { numParams = n; }
	int GetNumParams() const { return numParams; }
	void SetVariable() { variable = true; }
	bool GetVariable() const { return variable; }

	virtual void Compile(shared_ptr<Environment>& e, shared_ptr<SVM>& svm, BlockCnt& bc){
		SVM::Instruction nop(Opcode::NOP);

		int next = 0;
		SVM::Instruction jump(Opcode::JUMP, next);
		int jumpAddress = svm->AddCode(jump);
		int start = svm->AddCode(nop);
		SVM::Instruction reserve(Opcode::RESERVE, 0);
		int reserveAddress = svm->AddCode(reserve);
		Value func;
		func.SetFunction(start | (numParams << 24) | (variable << 31));
		int funcAddress = svm->AddGlobal(func);

		shared_ptr<Environment> local = shared_ptr<Environment>(new Environment());
		local->SetOutter(e);
		for (int i = 0; i < numParams; ++i){
			string name = children[i]->GetToken()->GetToken();
			if (name == "..."){
				name = "args";
				SymbolInfo si;
				si.address = numParams + 3;
				local->SetSymbol(name, si);
			}
			else{
				SymbolInfo si;
				si.address = i;
				local->SetSymbol(name, si);
			}
		}

		BlockCnt subBc;
		int numSpace = 4;
		subBc.variableIndex = numParams + numSpace;
		for (int i = numParams; i < children.size(); ++i){
			children[i]->Compile(local, svm, subBc);
		}
		reserve.operand = subBc.variableIndex - numParams - numSpace;
		svm->SetCode(reserveAddress, reserve);

		SVM::Instruction ret(Opcode::RET, numParams);
		for (int i = 0; i < subBc.rets.size(); ++i){
			int idx = subBc.rets[i];
			SVM::Instruction r = svm->GetCode(idx);
			r.operand |= (numParams & 0x0000ffff);
			svm->SetCode(idx, r);
		}

		int r = svm->AddCode(nop);
		if (subBc.rets.size() == 0 || subBc.rets[subBc.rets.size() - 1] != r - 1){
			svm->RemoveLastCode();
			svm->AddCode(ret);
		}
		else{
			svm->RemoveLastCode();
		}

		next = svm->AddCode(nop);
		jump.operand = next;
		svm->SetCode(jumpAddress, jump);

		SVM::Instruction push(Opcode::PUSH, funcAddress);
		svm->AddCode(push);
	}
};

SABER_NAMESPACE_END

#endif