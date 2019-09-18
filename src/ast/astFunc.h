#ifndef H_AST_ASTFUNC_H
#define H_AST_ASTFUNC_H

#include "../astree.h"
#include "astDef.h"

SABER_NAMESPACE_BEGIN

class AstFunc : public Astree{
private:
	bool fromFunc = false;
public:
	void SetFunc(bool f) { fromFunc = f; }

	virtual void Compile(shared_ptr<Environment>& e, shared_ptr<SVM>& svm, BlockCnt& bc){
		string funcName;
		if (!fromFunc){
			Token* tok = children[0]->GetToken();
			funcName = tok->GetToken();
			if (!istable && !e->HasSymbol(funcName)){
				Error::GetInstance()->ProcessError("����:%d, δ�����ʶ��[%s]\n", tok->GetLineNumber(), funcName.c_str());

				return;
			}
		}
		
		SymbolInfo si = e->GetSymbol(funcName);
		int func = si.address;
		int start = fromFunc ? 0 : 1;
		for (int j = start; j < children.size(); ++j){
			int numParams = children[j]->GetNumChildren();
			for (int i = 0; i < numParams; ++i){
				children[j]->GetChild(i)->Compile(e, svm, bc);
			}

			if (!fromFunc){
				if (j == start){
					if (!istable){
						SVM::Instruction push(Opcode::PUSH, func);
						svm->AddCode(push);
					}
					else{
						children[0]->Compile(e, svm, bc);
					}
				}
			}

			if (j != start){
				SVM::Instruction push(Opcode::PUSH, -numParams, true);
				svm->AddCode(push);
			}

			SVM::Instruction call(Opcode::CALL, numParams);
			svm->AddCode(call);
		}
	}
};


SABER_NAMESPACE_END

#endif