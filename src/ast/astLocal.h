#ifndef H_AST_ASTLOCAL_H
#define H_AST_ASTLOCAL_H

#include "../astree.h"

SABER_NAMESPACE_BEGIN

class AstLocal : public Astree{
public:
	virtual void Compile(shared_ptr<Environment>& e, shared_ptr<SVM>& svm, BlockCnt& bc){
		Token* tok;
		shared_ptr<Astree> node = children[0];
		while (node->GetNumChildren()) node = node->GetChild(0);
		
		tok = node->GetToken();
		if (tok->GetTokenType() != ETokenType::EIDENTIFIER){
			Error::GetInstance()->ProcessError("行数:%d, 变量名格式错误\n", tok->GetLineNumber());
			return;
		}

		int idx = bc.variableIndex;
		if (e->HasSymbol(tok->GetToken())){
			int level = 0;
			SymbolInfo si = e->GetSymbol(tok->GetToken(), level);
			if (level != 0 || !si.local){
				Value v;
				SymbolInfo si = { v, bc.variableIndex++, true };
				e->SetSymbol(tok->GetToken(), si);
				bc.nearst = idx;
				bc.nearstS = tok->GetToken();

				if (bc.cl){
					bc.cl->variables[tok->GetToken()] = idx;
				}
			}
			else{
				bc.nearst = si.address;
				bc.nearstS = tok->GetToken();
			}
		}
		else{
			Value v;
			SymbolInfo si = { v, bc.variableIndex++, true };
			e->SetSymbol(tok->GetToken(), si);
			bc.nearst = idx;
			bc.nearstS = tok->GetToken();

			if (bc.cl){
				bc.cl->variables[tok->GetToken()] = idx;
			}
		}
	}
};

SABER_NAMESPACE_END

#endif