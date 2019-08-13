#ifndef H_AST_ASTSTATEMENT_H
#define H_AST_ASTSTATEMENT_H

#include "../astree.h"

class AstStatement : public Astree{
public:
	virtual int Eval(shared_ptr<Environment>& e, shared_ptr<VM>& vm){
		int ret;
		for (int i = 0; i < children.size(); ++i){
			ret = children[i]->Eval(e, vm);
		}

		return ret;
	}
};

#endif