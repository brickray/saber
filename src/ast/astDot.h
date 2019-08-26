#ifndef H_AST_ASTDOT_H
#define H_AST_ASTDOT_H

#include "../astree.h"

SABER_NAMESPACE_BEGIN

class AstDot : public Astree{
public:
	virtual void Compile(shared_ptr<Environment>& e, shared_ptr<SVM>& svm, BlockCnt& bc){
		children[0]->Compile(e, svm, bc);
		children[1]->Compile(e, svm, bc);

		SVM::Instruction dot(Opcode::GTFILED);
		bc.nearst = svm->AddCode(dot);
	}
};

SABER_NAMESPACE_END

#endif