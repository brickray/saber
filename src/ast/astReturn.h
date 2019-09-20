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
			Error::GetInstance()->ProcessError("����:%d, �����ں������в���ʹ�ùؼ���[return]\n", token->GetLineNumber());

			return;
		}

		BlockCnt subBc;
		subBc.cl = bc.cl;
		subBc.maxLevel = bc.maxLevel;
		for (int i = 0; i < children.size(); ++i)
			children[i]->Compile(e, svm, subBc);

		SVM::Instruction tail(Opcode::TAILCALL);
		if (maybeTailCall && subBc.lasttail && !subBc.anyOperator){
			//return������ӵݹ���ò���û���κβ�����
			//��Ϊβ�ݹ�
			svm->SetCode(subBc.nearst, tail);
		}

		if (numRetParams == 0 && bc.lasttail && !bc.anytail){
			//���һ��Ϊ�ݹ���ã��ҽ�������return���
			//����û�з���ֵ��Ϊβ�ݹ�
			svm->SetCode(bc.nearst, tail);
		}

		bc.lasttail = false;

		SVM::Instruction ret(Opcode::RET, numRetParams);
		svm->AddCode(ret);
	}
};

SABER_NAMESPACE_END

#endif