#ifndef H_AST_ASTFUNC_H
#define H_AST_ASTFUNC_H

#include "../astree.h"
#include "astDef.h"

class AstFunc : public Astree{
public:
	virtual int Compile(shared_ptr<Environment>& e, shared_ptr<SVM>& svm){
		Token* tok = children[0]->GetToken();
		string funcName = tok->GetToken();
		if (!e->HasSymbol(funcName)){
			printf("行数:%d, 未定义函数名[%s]\n", tok->GetLineNumber(), funcName.c_str());
			return 0;
		}
		bool isFunc = e->GetSymbol(funcName).value.IsFunction();
		bool isNativeFunc = e->GetSymbol(funcName).value.IsNativeFunction();
		if (!isFunc && !isNativeFunc){
			printf("行数:%d,[%s]不是函数\n", tok->GetLineNumber(), funcName.c_str());
			return 0;
		}
		if (isFunc){
			SymbolInfo si = e->GetSymbol(funcName);
			int numParams = si.value.GetInteger();
			if (children.size() - 1 != numParams){
				printf("行数:%d, 函数参数不一致\n", tok->GetLineNumber());
				return 0;
			}

			SVM::Instruction call = { Opcode::CALL, si.address };
			int callAddress = svm->AddCode(call);
			call.operand1 = callAddress + 1;
			svm->SetCode(callAddress, call);
			
			return 0;
		}
		else{
			int func = e->GetSymbol(funcName).address;
			int numParams = children.size() - 1;
			vector<int> addresses(numParams);
			for (int i = 0; i < numParams; ++i){
				addresses[i] = children[i + 1]->Compile(e, svm);
			}

			SVM::Instruction call = { Opcode::CALLN, func, numParams };
			svm->AddCode(call);

			return 0;
		}
	}
};

#endif