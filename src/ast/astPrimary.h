#ifndef H_AST_ASTPRIMARY_H
#define H_AST_ASTPRIMARY_H

#include "../astree.h"

class AstPrimary : public Astree{
public:
	virtual int Compile(shared_ptr<Environment>& e, shared_ptr<SVM>& svm){
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

			return 0;
		}
		case ETokenType::EIDENTIFIER:
			if (e->HasSymbol(token->GetToken())){
				int idx = e->GetSymbol(token->GetToken()).address;

				SVM::Instruction ins = { Opcode::PUSH, idx, 0 };
				svm->AddCode(ins);
				return 0;
			}

			if (token->GetToken() == "true" ){
				int idx = -65;
				SVM::Instruction ins = { Opcode::PUSH, idx, 0 };
				svm->AddCode(ins);

				return 0;
			}
			else if (token->GetToken() == "false"){
				int idx = -66;
				SVM::Instruction ins = { Opcode::PUSH, idx, 0 };
				svm->AddCode(ins);

				return 0;
			}

			printf("行数:%d, 未定义标识符[%s]\n", token->GetLineNumber(), token->GetToken().c_str());
			break;
		case ETokenType::ESTRING:
			value.SetString(token->GetToken());

			int idx = svm->AddConstant(value);
			SVM::Instruction ins = { Opcode::PUSH, idx, 0 };
			svm->AddCode(ins);

			return 0;
		}
	}
};

#endif