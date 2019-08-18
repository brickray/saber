#ifndef H_AST_ASTRETURN_H
#define H_AST_ASTRETURN_H

#include "../astree.h"

SABER_NAMESPACE_BEGIN

class AstReturn : public Astree{
public:
	virtual void Compile(shared_ptr<Environment>& e, shared_ptr<SVM>& svm, BlockCnt& bc){
		if (bc.variableIndex == 0){
			Error::GetInstance()->ProcessError("行数:%d, 必需在函数体中才能使用关键字[return]\n", token->GetLineNumber());

			return;
		}

		for (int i = 0; i < children.size(); ++i)
			children[i]->Compile(e, svm, bc);

		SVM::Instruction ret = { Opcode::RET, 0 };
		ret.operand |= (children.size() << 16);
		int rp = svm->AddCode(ret);

		bc.rets.push_back(rp);
	}
};

SABER_NAMESPACE_END

#endif