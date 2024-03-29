#ifndef H_AST_ASTBERAK_H
#define H_AST_ASTBREAK_H

#include "../astree.h"

SABER_NAMESPACE_BEGIN

class AstBreak : public Astree{
public:
	virtual void Compile(shared_ptr<Environment>& e, shared_ptr<SVM>& svm, BlockCnt& bc){
		if (!bc.isloop){
			Error::GetInstance()->ProcessError("行数:%d, 必需在循环体中才能使用关键字[break]\n", token->GetLineNumber());

			return;
		}

		SVM::Instruction jump(Opcode::JUMP, 0);
		int bp = svm->AddCode(jump);

		bc.bps.push_back(bp);
	}
};

SABER_NAMESPACE_END

#endif