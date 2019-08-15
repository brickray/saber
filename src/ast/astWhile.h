#ifndef H_AST_ASTWHILE_H
#define H_AST_ASTWHILE_H

#include "../astree.h"

class AstWhile : public Astree{
public:
	virtual int Compile(shared_ptr<Environment>& e, shared_ptr<SVM>& svm, BlockCnt& bc){
		SVM::Instruction nop = { Opcode::NOP };
		int loopAddress = svm->AddCode(nop);
		children[0]->Compile(e, svm, bc);

		int end = 0;
		SVM::Instruction jz = { Opcode::JZ, end };
		int jumpAddress = svm->AddCode(jz);

		BlockCnt subBc;
		subBc.isloop = true;
		subBc.start = loopAddress;
		for (int i = 1; i < children.size(); ++i){
			children[i]->Compile(e, svm, subBc);
		}

		SVM::Instruction jump = { Opcode::JUMP, loopAddress };
		end = svm->AddCode(jump);
		jz.operand = end + 1;
		svm->SetCode(jumpAddress, jz);
		SVM::Instruction jumpb = { Opcode::JUMP, end + 1 };
		for (int i = 0; i < subBc.bps.size(); ++i){
			svm->SetCode(subBc.bps[i], jumpb);
		}

		return 0;
	}
};

#endif
