#ifndef H_AST_ASTWHILE_H
#define H_AST_ASTWHILE_H

#include "../astree.h"

class AstWhile : public Astree{
public:
	virtual int Compile(shared_ptr<Environment>& e, shared_ptr<SVM>& svm){
		SVM::Instruction nop = { Opcode::NOP };
		int loopAddress = svm->AddCode(nop);
		children[0]->Compile(e, svm);

		int end = 0;
		SVM::Instruction jz = { Opcode::JZ, end };
		int jumpAddress = svm->AddCode(jz);

		for (int i = 1; i < children.size(); ++i)
			children[i]->Compile(e, svm);

		SVM::Instruction jump = { Opcode::JUMP, loopAddress };
		end = svm->AddCode(jump);
		jz.operand = end + 1;
		svm->SetCode(jumpAddress, jz);

		return 0;
	}
};

#endif
