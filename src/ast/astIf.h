#ifndef H_AST_ASTIF_H
#define H_AST_ASTIF_H

#include "../astree.h"

SABER_NAMESPACE_BEGIN

class AstIf : public Astree{
private:
	bool hasElseBlock = false;

public:
	void SetElseBlock() {
		hasElseBlock = true;
	}

	virtual void Compile(shared_ptr<Environment>& e, shared_ptr<SVM>& svm, BlockCnt& bc){
		SVM::Instruction nop(Opcode::NOP);

		BlockCnt subBc;
		subBc.needRet = true;
		subBc.isloop = bc.isloop;
		subBc.bps = bc.bps;
		subBc.cps = bc.cps;
		subBc.cl = bc.cl;
		subBc.variableIndex = bc.variableIndex;
		subBc.maxLevel = bc.maxLevel;
		children[0]->Compile(e, svm, subBc);
		subBc.needRet = false;
		
		int next = 0;
		SVM::Instruction jz(Opcode::JZ, next);
		int jumpAddress = svm->AddCode(jz);
		children[1]->Compile(e, svm, subBc);
		SVM::Instruction jump(Opcode::JUMP, 0);
		int endAddress = svm->AddCode(jump);
		next = svm->AddCode(nop);
		svm->RemoveLastCode();
		jz.operand = next;
		svm->SetCode(jumpAddress, jz);

		int size = hasElseBlock ? children.size() - 1 : children.size();
		for (int i = 2; i < size; ++i){
			children[i]->Compile(e, svm, subBc);
		}

		if (hasElseBlock){
			children[children.size() - 1]->Compile(e, svm, subBc);
		}
		
		next = svm->AddCode(nop);
		svm->RemoveLastCode();
		jump.operand = next;
		svm->SetCode(endAddress, jump);

		for (int i = 0; i < subBc.elifs.size(); ++i){
			svm->SetCode(subBc.elifs[i], jump);
		}

		bc.bps = subBc.bps;
		bc.cps = subBc.cps;
		bc.variableIndex = subBc.variableIndex;
	}
};

SABER_NAMESPACE_END

#endif