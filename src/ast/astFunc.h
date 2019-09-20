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
				Error::GetInstance()->ProcessError("行数:%d, 未定义标识符[%s]\n", tok->GetLineNumber(), funcName.c_str());

				return;
			}
		}
		
		bool lasttail = false;
		bool anyOperator = bc.anyOperator;
		SymbolInfo si = e->GetSymbol(funcName);
		if (!fromFunc && children.size() == 2){
			//尾递归不支持可变参
			if (si.value.IsFunction() && si.value.GetFunction() == bc.cl && !bc.cl->vararg){
				lasttail = true;
			}
		}
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
				else{
					SVM::Instruction push(Opcode::PUSH, numParams, true);
					svm->AddCode(push);
				}
			}
			else{
				SVM::Instruction push(Opcode::PUSH, numParams, true);
				svm->AddCode(push);
			}

			SVM::Instruction call(Opcode::CALL, numParams);
			int address = svm->AddCode(call);
			bc.nearst = address;
		}

		bc.lasttail = lasttail;
		bc.anyOperator = anyOperator;
	}
};


SABER_NAMESPACE_END

#endif