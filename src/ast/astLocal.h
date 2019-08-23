#ifndef H_AST_ASTLOCAL_H
#define H_AST_ASTLOCAL_H

#include "../astree.h"

SABER_NAMESPACE_BEGIN

class AstLocal : public Astree{
public:
	virtual void Compile(shared_ptr<Environment>& e, shared_ptr<SVM>& svm, BlockCnt& bc){
		Token* tok = children[0]->GetToken();
		if (tok->GetTokenType() != ETokenType::EIDENTIFIER){
			Error::GetInstance()->ProcessError("行数:%d, 变量名格式错误\n", tok->GetLineNumber());
			return;
		}

		int idx = bc.variableIndex;
		Value v;
		SymbolInfo si = { v, bc.variableIndex++, true };
		e->SetSymbol(tok->GetToken(), si);
		bc.nearst = idx;
	}
};

SABER_NAMESPACE_END

#endif