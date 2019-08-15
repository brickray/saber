#ifndef H_AST_ASTBERAK_H
#define H_AST_ASTBREAK_H

#include "../astree.h"

SABER_NAMESPACE_BEGIN

class AstBreak : public Astree{
public:
	virtual int Compile(shared_ptr<Environment>& e, shared_ptr<SVM>& svm, BlockCnt& bc){
		if (!bc.isloop){
			printf("行数:%d, 必需在循环体中才能使用关键字[break]\n", token->GetLineNumber());
			return 0;
		}

		SVM::Instruction jump = { Opcode::JUMP, bc.start };
		int bp = svm->AddCode(jump);

		bc.bps.push_back(bp);
		return 0;
	}
};

SABER_NAMESPACE_END

#endif