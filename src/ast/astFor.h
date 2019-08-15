#ifndef H_AST_ASTFOR_H
#define H_AST_ASTFOR_H

#include "../astree.h"

SABER_NAMESPACE_BEGIN

class AstFor : public Astree{
public:
	virtual void Compile(shared_ptr<Environment>& e, shared_ptr<SVM>& svm, BlockCnt& bc){
		SVM::Instruction nop = { Opcode::NOP };
		children[0]->Compile(e, svm, bc);

		int loopAddress = svm->AddCode(nop);
		children[1]->Compile(e, svm, bc);
		SVM::Instruction jz = { Opcode::JZ };
		int jumpAddress = svm->AddCode(jz);

		BlockCnt subBc;
		subBc.start = loopAddress;
		subBc.isloop = true;
		children[children.size() - 1]->Compile(e, svm, subBc);

		if (children.size() == 4){//has step component
			children[2]->Compile(e, svm, bc);
		}

		SVM::Instruction jump = { Opcode::JUMP, loopAddress };
		int end = svm->AddCode(jump);
		jz.operand = end + 1;
		svm->SetCode(jumpAddress, jz);

		SVM::Instruction jumpb = { Opcode::JUMP, end + 1 };
		for (int i = 0; i < subBc.bps.size(); ++i){
			svm->SetCode(subBc.bps[i], jumpb);
		}
	}
};

SABER_NAMESPACE_END

#endif