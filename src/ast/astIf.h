#ifndef H_AST_ASTIF_H
#define H_AST_ASTIF_H

#include "../astree.h"

class AstIf : public Astree{
private:
	bool hasElseBlock = false;

public:
	void SetElseBlock() {
		hasElseBlock = true;
	}

	virtual int Eval(shared_ptr<Environment>& e, shared_ptr<VM>& vm){
		SValue sv;
		Value value;
		value.SetType(EValueType::EBOOLEAN);
		/*Value cond = children[0]->Eval(e, vm);
		if ((cond.IsBoolean() && !cond.GetBoolean()) || 
			(cond.IsInteger() && cond.GetInteger()) ||
			(cond.IsFloat() && cond.GetFloat())){

			int size = hasElseBlock ? children.size() - 1 : children.size();
			for (int i = 2; i < size; ++i){
				Value ret = children[i]->Eval(e, vm);
				if (ret.GetBoolean()) return value;
			}

			if (hasElseBlock){
				children[children.size() - 1]->Eval(e, vm);
			}

			sv.bValue = false;
			value.SetValue(sv);
			return value;
		}*/

		children[1]->Eval(e, vm);
		sv.bValue = true;
		value.SetValue(sv);
		return 0;
	}
};

#endif