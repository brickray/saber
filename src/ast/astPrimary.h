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

			int idx = svm->AddConstant(value);
			SVM::Instruction ins = { Opcode::PUSH, idx, 0 };
			svm->AddCode(ins);
			break;
		}
		case ETokenType::EIDENTIFIER:
			if (token->GetToken() == "true"){
				int idx = -1;
				SVM::Instruction ins = { Opcode::PUSH, idx, 0 };
				svm->AddCode(ins);
				return;
			}
			else if (token->GetToken() == "false"){
				int idx = -2;
				SVM::Instruction ins = { Opcode::PUSH, idx, 0 };
				svm->AddCode(ins);
				return;
			}

			if (e->HasSymbol(token->GetToken())){
				int idx = e->GetSymbol(token->GetToken()).address;

				SVM::Instruction ins = { Opcode::PUSH, idx, 0 };
				svm->AddCode(ins);

				bc.nearst = idx;
				return;
			}

			printf("行数:%d, 未定义标识符[%s]\n", token->GetLineNumber(), token->GetToken().c_str());
			break;
		case ETokenType::ESTRING:
			value.SetString(token->GetToken());

			int idx = svm->AddConstant(value);
			SVM::Instruction ins = { Opcode::PUSH, idx, 0 };
			svm->AddCode(ins);
			break;
		}
	}
};

SABER_NAMESPACE_END

#endif