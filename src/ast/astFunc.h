#ifndef H_AST_ASTFUNC_H
#define H_AST_ASTFUNC_H

#include "../astree.h"
#include "astDef.h"

class AstFunc : public Astree{
public:
	virtual int Compile(shared_ptr<Environment>& e, shared_ptr<SVM>& svm, BlockCnt& bc){
		Token* tok = children[0]->GetToken();
		string funcName = tok->GetToken();
		if (!e->HasSymbol(funcName)){
			printf("行数:%d, 未定义标识符[%s]\n", tok->GetLineNumber(), funcName.c_str());
			return 0;
		}
		
		int func = e->GetSymbol(funcName).address;
		int numParams = children.size() - 1;
		for (int i = 0; i < numParams; ++i){
			children[i + 1]->Compile(e, svm, bc);
		}

		SVM::Instruction push = { Opcode::PUSH, func };
		svm->AddCode(push);
		SVM::Instruction call = { Opcode::CALL, numParams };
		int callAddress = svm->AddCode(call);
		call.operand1 = callAddress + 1;
		svm->SetCode(callAddress, call);

		return 0;
	}
};

#endif