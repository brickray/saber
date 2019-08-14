#ifndef H_AST_ASTSTATEMENT_H
#define H_AST_ASTSTATEMENT_H

#include "../astree.h"

class AstStatement : public Astree{
public:
	virtual int Compile(shared_ptr<Environment>& e, shared_ptr<SVM>& vm){
		int ret = 0;
		for (int i = 0; i < children.size(); ++i){
			children[i]->Compile(e, vm);
		}

		return ret;
	}
};

#endif