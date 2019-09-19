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
		subBc.ret = true;
		subBc.maxLevel = bc.maxLevel;
		for (int i = 0; i < children.size(); ++i)
			children[i]->Compile(e, svm, subBc);

		if (maybeTailCall && subBc.tailcall && numRetParams != 0){
			SVM::Instruction tail(Opcode::TAILCALL);
			svm->SetCode(subBc.nearst, tail);
		}

		SVM::Instruction ret(Opcode::RET, numRetParams);
		int rp = svm->AddCode(ret);
	}
};

SABER_NAMESPACE_END

#endif