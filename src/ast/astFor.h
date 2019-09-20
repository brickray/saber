#ifndef H_AST_ASTFOR_H
#define H_AST_ASTFOR_H

#include "../astree.h"

SABER_NAMESPACE_BEGIN

class AstFor : public Astree{
public:
	virtual void Compile(shared_ptr<Environment>& e, shared_ptr<SVM>& svm, BlockCnt& bc){
		SVM::Instruction nop(Opcode::NOP);
		children[0]->Compile(e, svm, bc);

		int loopAddress = svm->AddCode(nop);
		svm->RemoveLastCode();

		BlockCnt subBc;
		subBc.isloop = true;
		subBc.cl = bc.cl;
		subBc.variableIndex = bc.variableIndex;
		subBc.maxLevel = bc.maxLevel;
		children[1]->Compile(e, svm, bc);
		SVM::Instruction jz(Opcode::JZ);
		int jumpAddress = svm->AddCode(jz);

		children[children.size() - 1]->Compile(e, svm, subBc);

		int continueAddress = svm->AddCode(nop);
		svm->RemoveLastCode();
		if (children.size() == 4){//has step component
			children[2]->Compile(e, svm, subBc);
		}

		SVM::Instruction jump(Opcode::JUMP, loopAddress);
		int end = svm->AddCode(jump);
		jz.operand = end + 1;
		svm->SetCode(jumpAddress, jz);

		SVM::Instruction jumpb(Opcode::JUMP, end + 1);
		for (int i = 0; i < subBc.bps.size(); ++i){
			svm->SetCode(subBc.bps[i], jumpb);
		}
		SVM::Instruction jumpc(Opcode::JUMP, continueAddress);
		for (int i = 0; i < subBc.cps.size(); ++i){
			svm->SetCode(subBc.cps[i], jumpc);
		}

		bc.variableIndex = subBc.variableIndex;
	}
};

SABER_NAMESPACE_END

#endif