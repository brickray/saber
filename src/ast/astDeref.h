#ifndef H_AST_ASTDEREF_H
#define H_AST_ASTDEREF_H

#include "../astree.h"

SABER_NAMESPACE_BEGIN

class AstDeref : public Astree{
public:
	virtual void Compile(shared_ptr<Environment>& e, shared_ptr<SVM>& svm, BlockCnt& bc){
		children[0]->Compile(e, svm, bc);

		SVM::Instruction c(Opcode::DEREF);
		svm->AddCode(c);
	}
};

SABER_NAMESPACE_END

#endif