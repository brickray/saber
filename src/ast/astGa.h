#ifndef H_AST_ASTGA_H
#define H_AST_ASTGA_H

#include "../astree.h"

SABER_NAMESPACE_BEGIN

class AstGa : public Astree{
public:
	virtual void Compile(shared_ptr<Environment>& e, shared_ptr<SVM>& svm, BlockCnt& bc){
		bc.getad = true;
		children[0]->Compile(e, svm, bc);
		bc.getad = false;
	}
};

SABER_NAMESPACE_END

#endif