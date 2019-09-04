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
				float v = atof(tok.c_str());

				SVM::Instruction ins(Opcode::PUSHF, v);
				svm->AddCode(ins);
			}
			else{
				int v = atoi(tok.c_str());

				SVM::Instruction ins(Opcode::PUSHI, v);
				svm->AddCode(ins);
			}
			break;
		}
		case ETokenType::EIDENTIFIER:{
			string tok = token->GetToken();
			if (tok == "true" || tok == "false"){
				int i = tok == "true" ? 1 : 0;
				SVM::Instruction ins(Opcode::PUSHB, i);
				svm->AddCode(ins);
				return;
			}

			if (e->HasSymbol(tok)){
				int level = 0;
				int idx = e->GetSymbol(tok, level).address;
				int p = idx;
				if (level != 0 && level < bc.maxLevel){
					idx |= (level << 24);
					int i = 0;
					for (; i < bc.cl->cvs.size(); ++i){
						if (bc.cl->cvs[i] == idx)
							break;
					}
					if (i == bc.cl->cvs.size()){
						int size = bc.cl->cvs.size();
						bc.cl->cvs.push_back(idx);
						//closure value
						p = size;
						p |= (1 << 30);
					}
					else{
						p = i |= (1 << 30);
					}
				}

				SVM::Instruction ins(Opcode::PUSH, p, tok);
				svm->AddCode(ins);

				bc.nearst = idx;
				bc.nearstS = tok;
				return;
			}

			Error::GetInstance()->ProcessError("行数:%d, 未定义标识符[%s]\n", token->GetLineNumber(), token->GetToken().c_str());
			break;
		}
		case ETokenType::ESTRING:{
			string str = token->GetToken();

			SVM::Instruction ins(Opcode::PUSHS, str);
			svm->AddCode(ins);
			break;
		}
		}
	}
};

SABER_NAMESPACE_END

#endif