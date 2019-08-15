#ifndef H_AST_ASTSTATEMENT_H
#define H_AST_ASTSTATEMENT_H

#include "../astree.h"

SABER_NAMESPACE_BEGIN

class AstStatement : public Astree{
public:
	virtual int Compile(shared_ptr<Environment>& e, shared_ptr<SVM>& vm, BlockCnt& bc){
		for (int i = 0; i < children.size(); ++i){
			children[i]->Compile(e, vm, bc);
		}

		return 0;
	}
};

SABER_NAMESPACE_END

#endif