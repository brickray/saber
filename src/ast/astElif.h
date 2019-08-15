#ifndef H_AST_ASTELIF_H
#define H_AST_ASTELIF_H

#include "../astree.h"

SABER_NAMESPACE_BEGIN

class AstElif : public Astree{
public:
	virtual void Compile(shared_ptr<Environment>& e, shared_ptr<SVM>& svm, BlockCnt& bc){
		SVM::Instruction nop = { Opcode::NOP };

		children[0]->Compile(e, svm, bc);

		int next = 0;
		SVM::Instruction jz = { Opcode::JZ, next };
		int jumpAddress = svm->AddCode(jz);
		children[1]->Compile(e, svm, bc);
		SVM::Instruction jump = { Opcode::JUMP, 0 };
		int endAddress = svm->AddCode(jump);
		next = svm->AddCode(nop);
		jz.operand = next;
		svm->SetCode(jumpAddress, jz);

		bc.elifs.push_back(endAddress);
	}
};

SABER_NAMESPACE_END

#endif