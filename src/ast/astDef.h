#ifndef H_AST_ASTDEF_H
#define H_AST_ASTDEF_H

#include "../astree.h"

class AstDef : public Astree{
private:
	int numReturnParams;
public:
	void SetNumReturnParams(int n) { numReturnParams = n; }
	int GetNumReturnParams() const { return numReturnParams; }

	virtual int Eval(shared_ptr<Environment>& e, shared_ptr<VM>& vm){
		SValue sv;
		Value v;
		sv.func = shared_ptr<Astree>(this);
		v.SetType(EValueType::EFUNC);
		v.SetValue(sv);
		SymbolInfo si = { v, 0 };
		e->SetSymbol(children[0]->GetToken()->GetToken(), si);

		return 0;
	}
};

#endif