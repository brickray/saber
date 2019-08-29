#ifndef H_AST_ASTFUNC_H
#define H_AST_ASTFUNC_H

#include "../astree.h"
#include "astDef.h"

SABER_NAMESPACE_BEGIN

class AstFunc : public Astree{
public:
	virtual void Compile(shared_ptr<Environment>& e, shared_ptr<SVM>& svm, BlockCnt& bc){
		Token* tok = children[0]->GetToken();
		string funcName = tok->GetToken();
		if (!istable && !e->HasSymbol(funcName)){
			Error::GetInstance()->ProcessError("行数:%d, 未定义标识符[%s]\n", tok->GetLineNumber(), funcName.c_str());

			return;
		}
		
		SymbolInfo si = e->GetSymbol(funcName);
		int func = si.address;
		for (int j = 1; j < children.size(); ++j){
			int numParams = children[j]->GetNumChildren();
			for (int i = 0; i < numParams; ++i){
				children[j]->GetChild(i)->Compile(e, svm, bc);
			}

			if (!istable){
				if (j == 1){
					SVM::Instruction push(Opcode::PUSH, func);
					svm->AddCode(push);
				}
				else{
					SVM::Instruction push(Opcode::PUSH, -numParams, true);
					svm->AddCode(push);
				}
			}
			else{
				if (j == 1){
					children[0]->Compile(e, svm, bc);
				}
				else{
					SVM::Instruction push(Opcode::PUSH, -numParams, true);
					svm->AddCode(push);
				}
			}

			SVM::Instruction call(Opcode::CALL, numParams);
			svm->AddCode(call);
		}
	}
};


SABER_NAMESPACE_END

#endif