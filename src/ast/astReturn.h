#ifndef H_AST_ASTRETURN_H
#define H_AST_ASTRETURN_H

#include "../astree.h"

SABER_NAMESPACE_BEGIN

class AstReturn : public Astree{
private:
	bool numRetParams = 0;

public:
	void SetNumRetParams(int n){ numRetParams = n; }
	int GetNumRetParams() const { return numRetParams; }

	virtual void Compile(shared_ptr<Environment>& e, shared_ptr<SVM>& svm, BlockCnt& bc){
		if (!e->GetOutter()){
			Error::GetInstance()->ProcessError("����:%d, �����ں������в���ʹ�ùؼ���[return]\n", token->GetLineNumber());

			return;
		}

		for (int i = 0; i < children.size(); ++i)
			children[i]->Compile(e, svm, bc);

		SVM::Instruction ret(Opcode::RET, 0);
		ret.operand |= (numRetParams << 16);
		int rp = svm->AddCode(ret);

		bc.rets.push_back(rp);
	}
};

SABER_NAMESPACE_END

#endif