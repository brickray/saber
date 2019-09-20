#ifndef H_AST_ASTRETURN_H
#define H_AST_ASTRETURN_H

#include "../astree.h"

SABER_NAMESPACE_BEGIN

class AstReturn : public Astree{
private:
	int numRetParams = 0;
	bool maybeTailCall = false;

public:
	void SetNumRetParams(int n){ numRetParams = n; }
	int GetNumRetParams() const { return numRetParams; }
	void SetMaybeTailCall() { maybeTailCall = true; }

	virtual void Compile(shared_ptr<Environment>& e, shared_ptr<SVM>& svm, BlockCnt& bc){
		if (!e->GetOutter()){
			Error::GetInstance()->ProcessError("行数:%d, 必需在函数体中才能使用关键字[return]\n", token->GetLineNumber());

			return;
		}

		BlockCnt subBc;
		subBc.cl = bc.cl;
		subBc.maxLevel = bc.maxLevel;
		for (int i = 0; i < children.size(); ++i)
			children[i]->Compile(e, svm, subBc);

		SVM::Instruction tail(Opcode::TAILCALL);
		if (maybeTailCall && subBc.lasttail && !subBc.anyOperator){
			//return语句后面接递归调用并且没有任何操作符
			//则为尾递归
			svm->SetCode(subBc.nearst, tail);
		}

		if (numRetParams == 0 && bc.lasttail && !bc.anytail){
			//最近一次为递归调用，且接下来是return语句
			//并且没有返回值，为尾递归
			svm->SetCode(bc.nearst, tail);
		}

		bc.lasttail = false;

		SVM::Instruction ret(Opcode::RET, numRetParams);
		svm->AddCode(ret);
	}
};

SABER_NAMESPACE_END

#endif