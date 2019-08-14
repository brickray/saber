#ifndef H_AST_ASTDEF_H
#define H_AST_ASTDEF_H

#include "../astree.h"

class AstDef : public Astree{
private:
	int numParams;
	int numReturnParams;
public:
	void SetNumParams(int n) { numParams = n; }
	int GetNumParams() const { return numParams; }
	void SetNumReturnParams(int n) { numReturnParams = n; }
	int GetNumReturnParams() const { return numReturnParams; }

	virtual int Compile(shared_ptr<Environment>& e, shared_ptr<SVM>& svm){
		SVM::Instruction nop = { Opcode::NOP };

		int next = 0;
		SVM::Instruction jump = { Opcode::JUMP, next };
		int jumpAddress = svm->AddCode(jump);
		int start = svm->AddCode(nop);
		Value v;
		v.SetInt(numParams);
		v.SetType(EValueType::EFUNC);
		SymbolInfo si = { v, start };
		e->SetSymbol(children[0]->GetToken()->GetToken(), si);



		for (int i = numParams + 1; i < children.size() - numReturnParams; ++i){
			children[i]->Compile(e, svm);
		}

		SVM::Instruction ret = { Opcode::RET };
		svm->AddCode(ret);

		next = svm->AddCode(nop);
		jump.operand = next;
		svm->SetCode(jumpAddress, jump);

		return 0;
	}
};

#endif