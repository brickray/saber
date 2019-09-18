#ifndef H_AST_ASTDEF_H
#define H_AST_ASTDEF_H

#include "../astree.h"

SABER_NAMESPACE_BEGIN

class AstDef : public Astree{
private:
	int numParams = 0;
public:
	void SetNumParams(int n) { numParams = n; }
	int GetNumParams() const { return numParams; }

	virtual void Compile(shared_ptr<Environment>& e, shared_ptr<SVM>& svm, BlockCnt& bc){
		SVM::Instruction nop(Opcode::NOP);

		int next = 0;
		SVM::Instruction jump(Opcode::JUMP, next);
		int jumpAddress = svm->AddCode(jump);
		int start = jumpAddress + 1;
		SVM::Instruction reserve(Opcode::RESERVE, 0);
		int reserveAddress = svm->AddCode(reserve);
		Value func;
		//set closure
		Clptr cl = Clptr(new Closure());
		cl->closure = false;
		cl->hascv = false;
		cl->vararg = false;
		cl->entry = start;
		cl->fp = numParams;
		cl->parent = bc.cl;
		func.SetFunction(cl);
		int funcAddress = svm->AddGlobal(func);
		SymbolInfo si = { func, funcAddress };
		e->SetSymbol(children[0]->GetToken()->GetToken(), si);
		if (bc.cl) bc.cl->cls.insert(cl);

		//set environment
		shared_ptr<Environment> local = shared_ptr<Environment>(new Environment());
		local->SetOutter(e);
		for (int i = 1; i < numParams + 1; ++i){
			string name = children[i]->GetToken()->GetToken();
			if (name == "..."){
				name = "args";
				SymbolInfo si;
				si.address = numParams + 7;
				local->SetSymbol(name, si);

				cl->variables[name] = numParams + 7;
				cl->vararg = true;
			}
			else{
				SymbolInfo si;
				si.address = i - 1;
				local->SetSymbol(name, si);

				cl->variables[name] = i - 1;
			}
		}

		BlockCnt subBc;
		subBc.cl = cl;
		subBc.maxLevel = bc.maxLevel + 1;
		int numSpace = 8;
		subBc.variableIndex = numParams + numSpace;
		for (int i = numParams + 1; i < children.size(); ++i){
			children[i]->Compile(local, svm, subBc);
		}
		reserve.operand = subBc.variableIndex - numParams - numSpace;
		svm->SetCode(reserveAddress, reserve);

		SVM::Instruction ret(Opcode::RET, numParams);
		SVM::Instruction last = svm->GetLastCode();
		if (last.opcode != Opcode::RET){
			svm->AddCode(ret);
		}

		next = svm->AddCode(nop);
		svm->RemoveLastCode();
		jump.operand = next;
		svm->SetCode(jumpAddress, jump);
	}
};

SABER_NAMESPACE_END

#endif