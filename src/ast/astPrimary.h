#ifndef H_AST_ASTPRIMARY_H
#define H_AST_ASTPRIMARY_H

#include "../astree.h"

class AstPrimary : public Astree{
public:
	virtual int Eval(shared_ptr<Environment>& e, shared_ptr<VM>& vm){
		EValueType vt;
		SValue sv;
		Value value;
		ETokenType type = token->GetTokenType();
		switch (type){
		case ETokenType::ENUMBER:{
			string tok = token->GetToken();
			if (tok.find('.') != -1){
				value.SetType(EValueType::EFLOAT);
				float v = atof(tok.c_str());
				sv.fValue = v;
			}
			else{
				value.SetType(EValueType::EINTEGER);
				int v = atoi(tok.c_str());
				sv.iValue = v;
			}

			value.SetValue(sv);

			int idx = vm->AddConstant(value);

			return idx;
		}
		case ETokenType::EIDENTIFIER:
			if (e->HasSymbol(token->GetToken())){
				int idx = e->GetSymbol(token->GetToken()).address;
				return idx;
			}

			if (token->GetToken() == "true" ){
				sv.bValue = true;
				value.SetType(EValueType::EBOOLEAN);
				value.SetValue(sv);

				int idx = vm->AddConstant(value);

				return idx;
			}
			else if (token->GetToken() == "false"){
				sv.bValue = false;
				value.SetType(EValueType::EBOOLEAN);
				value.SetValue(sv);

				int idx = vm->AddConstant(value);

				return idx;
			}

			printf("行数:%d, 未定义标识符[%s]\n", token->GetLineNumber(), token->GetToken().c_str());
			break;
		case ETokenType::ESTRING:
			value.SetType(EValueType::ESTRING);
			sv.sValue = token->GetToken();
			value.SetValue(sv);

			int idx = vm->AddConstant(value);
			return idx;
		}
	}
};

#endif