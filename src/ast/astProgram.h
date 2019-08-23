#ifndef H_AST_ASTPROGRAM_H
#define H_AST_ASTPROGRAM_H

#include "../astree.h"

SABER_NAMESPACE_BEGIN

class AstProgram : public Astree{
public:
	virtual void Compile(shared_ptr<Environment>& e, shared_ptr<SVM>& svm, BlockCnt& bc){
		SVM::Instruction reserve(Opcode::RESERVE);
		int reserveAddress = svm->AddCode(reserve);

		for (int i = 0; i < children.size(); ++i)
			children[i]->Compile(e, svm, bc);

		reserve.operand = bc.variableIndex;
		svm->SetCode(reserveAddress, reserve);
	}
};

SABER_NAMESPACE_END

#endif