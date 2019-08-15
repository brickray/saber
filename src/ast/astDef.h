#ifndef H_AST_ASTDEF_H
#define H_AST_ASTDEF_H

#include "../astree.h"

SABER_NAMESPACE_BEGIN

class AstDef : public Astree{
private:
	int numParams;
	int numReturnParams;
public:
	void SetNumParams(int n) { numParams = n; }
	int GetNumParams() const { return numParams; }
	void SetNumReturnParams(int n) { numReturnParams = n; }
	int GetNumReturnParams() const { return numReturnParams; }

	virtual void Compile(shared_ptr<Environment>& e, shared_ptr<SVM>& svm, BlockCnt& bc){
		SVM::Instruction nop = { Opcode::NOP };

		int next = 0;
		SVM::Instruction jump = { Opcode::JUMP, next };
		int jumpAddress = svm->AddCode(jump);
		int start = svm->AddCode(nop);
		Value func;
		func.SetInt(start);
		func.SetType(EValueType::EFUNC);
		int funcAddress = svm->AddGlobal(func);
		SymbolInfo si = { func, funcAddress };
		e->SetSymbol(children[0]->GetToken()->GetToken(), si);

		for (int i = numParams + 1; i < children.size() - numReturnParams; ++i){
			children[i]->Compile(e, svm, bc);
		}

		SVM::Instruction ret = { Opcode::RET, numParams };
		svm->AddCode(ret);

		next = svm->AddCode(nop);
		jump.operand = next;
		svm->SetCode(jumpAddress, jump);
	}
};

SABER_NAMESPACE_END

#endif