#ifndef H_AST_ASTPRIMARY_H
#define H_AST_ASTPRIMARY_H

#include "../astree.h"

SABER_NAMESPACE_BEGIN

class AstPrimary : public Astree{
public:
	virtual void Compile(shared_ptr<Environment>& e, shared_ptr<SVM>& svm, BlockCnt& bc){
		EValueType vt;
		Value value;
		ETokenType type = token->GetTokenType();
		switch (type){
		case ETokenType::ENUMBER:{
			string tok = token->GetToken();
			int idx;
			if (e->HasNumberSymbol(tok)){
				idx = e->GetNumberSymbol(tok).address;
			}
			else{
				if (tok.find('.') != -1){
					value.SetType(EValueType::EFLOAT);
					float v = atof(tok.c_str());
					value.SetFloat(v);
				}
				else{
					value.SetType(EValueType::EINTEGER);
					int v = atoi(tok.c_str());
					value.SetInt(v);
				}

				idx = svm->AddConstant(value);
				SymbolInfo si = { value, idx, false };
				e->SetNumberSymbol(tok, si);
			}
			
			SVM::Instruction ins(Opcode::PUSH, idx);
			svm->AddCode(ins);
			break;
		}
		case ETokenType::EIDENTIFIER:
			if (token->GetToken() == "true"){
				int idx = -1;
				SVM::Instruction ins(Opcode::PUSH, idx);
				svm->AddCode(ins);
				return;
			}
			else if (token->GetToken() == "false"){
				int idx = -2;
				SVM::Instruction ins(Opcode::PUSH, idx);
				svm->AddCode(ins);
				return;
			}

			if (e->HasSymbol(token->GetToken())){
				int idx = e->GetSymbol(token->GetToken()).address;

				SVM::Instruction ins(Opcode::PUSH, idx);
				svm->AddCode(ins);

				bc.nearst = idx;
				return;
			}

			Error::GetInstance()->ProcessError("行数:%d, 未定义标识符[%s]\n", token->GetLineNumber(), token->GetToken().c_str());
			break;
		case ETokenType::ESTRING:
			string str = token->GetToken();
			int idx;
			if (e->HasStringSymbol(str)){
				idx = e->GetStringSymbol(str).address;
			}
			else{
				value.SetString(str);
				idx = svm->AddConstant(value);
				SymbolInfo si = { value, idx, false };
				e->SetStringSymbol(str, si);
			}

			SVM::Instruction ins(Opcode::PUSH, idx);
			svm->AddCode(ins);
			break;
		}
	}
};

SABER_NAMESPACE_END

#endif