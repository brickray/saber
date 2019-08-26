#ifndef H_AST_ASTTABLE_H
#define H_AST_ASTTABLE_H

#include "../astree.h"

SABER_NAMESPACE_BEGIN

class AstTable : public Astree{
	virtual void Compile(shared_ptr<Environment>& e, shared_ptr<SVM>& svm, BlockCnt& bc){
		SVM::Instruction s(Opcode::SETTABLE);
		svm->AddCode(s);
	}
};

SABER_NAMESPACE_END

#endif