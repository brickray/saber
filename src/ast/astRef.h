#ifndef H_AST_ASTREF_H
#define H_AST_ASTREF_H

#include "../astree.h"

SABER_NAMESPACE_BEGIN

class AstRef : public Astree{
public:
	virtual void Compile(shared_ptr<Environment>& e, shared_ptr<SVM>& svm, BlockCnt& bc){
		bc.ref = true;
		children[0]->Compile(e, svm, bc);
		bc.ref = false;
	}
};

SABER_NAMESPACE_END

#endif