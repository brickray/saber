#ifndef H_AST_ASTIF_H
#define H_AST_ASTIF_H

#include "../astree.h"

class AstIf : public Astree{
private:
	bool hasElseBlock = false;

public:
	void SetElseBlock() {
		hasElseBlock = true;
	}

	virtual int Compile(shared_ptr<Environment>& e, shared_ptr<SVM>& svm){
		SVM::Instruction nop = { Opcode::NOP };

		children[0]->Compile(e, svm);
		
		int next = 0;
		SVM::Instruction jz = { Opcode::JZ, next };
		int jumpAdress = svm->AddCode(jz);
		children[1]->Compile(e, svm);
		SVM::Instruction jump = { Opcode::JUMP, 0 };
		int endAdress = svm->AddCode(jump);
		next = svm->AddCode(nop);
		jz.operand = next;
		svm->SetCode(jumpAdress, jz);

		int size = hasElseBlock ? children.size() - 1 : children.size();
		for (int i = 2; i < size; ++i){
			children[i]->Compile(e, svm);
		}

		if (hasElseBlock){
			children[children.size() - 1]->Compile(e, svm);
			next = svm->AddCode(nop);
			jump.operand = next;
			svm->SetCode(endAdress, jump);
		}
		else{
			jump.operand = next;
			svm->SetCode(endAdress, jump);
		}

		return 0;
	}
};

#endif