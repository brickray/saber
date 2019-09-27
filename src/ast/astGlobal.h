#ifndef H_AST_ASTGLOBAL_H
#define H_AST_ASTGLOBAL_H

#include "../astree.h"

SABER_NAMESPACE_BEGIN

class AstGlobal : public Astree{
public:
	virtual void Compile(shared_ptr<Environment>& e, shared_ptr<SVM>& svm, BlockCnt& bc){
		Token* tok = children[0]->GetToken();
		if (istable){
			shared_ptr<Astree> node = children[0], left = node;
			while (node->GetNumChildren()){
				node = node->GetChild(0);
				left = node;
			}
			tok = left->GetToken();
		}
		if (tok->GetTokenType() != ETokenType::EIDENTIFIER){
			Error::GetInstance()->ProcessError("行数:%d, 变量名格式错误\n", tok->GetLineNumber());
			return;
		}

		if (istable){
			children[0]->Compile(e, svm, bc);
			return;
		}

		int idx;
		if (!e->HasSymbol(tok->GetToken())){
			Value v;
			idx = svm->AddGlobal(v);
			SymbolInfo si = { v, idx, false };
			e->SetSymbol(tok->GetToken(), si);
			bc.nearst = idx;
			bc.nearstS = tok->GetToken();

			if (bc.cl){
				bc.cl->variables[tok->GetToken()] = idx;
			}
		}
		else{
			int level = 0;
			SymbolInfo si = e->GetSymbol(tok->GetToken(), level);
			bool local = si.local;
			if (level != 0){
				if (level < bc.maxLevel){
					bc.cl->hascv = true;
					bc.nearst = 1 << 30;
					bc.nearstS = tok->GetToken();
				}
				else{
					if (local){
						Value v;
						idx = svm->AddGlobal(v);
						SymbolInfo si = { v, idx, false };
						e->SetSymbol(tok->GetToken(), si);
						bc.nearst = idx;
						bc.nearstS = tok->GetToken();
					}
					else{
						bc.nearst = si.address;
						bc.nearstS = tok->GetToken();
					}
				}
			}
			else{
				bc.nearst = si.address;
				bc.nearstS = tok->GetToken();
			}
		}
	}
};

SABER_NAMESPACE_END

#endif