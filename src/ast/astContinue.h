#ifndef H_AST_ATSCONTINUE_H
#define H_AST_ASTCONTINUE_H

#include "../astree.h"

SABER_NAMESPACE_BEGIN

class AstContinue : public Astree{
public:
	virtual void Compile(shared_ptr<Environment>& e, shared_ptr<SVM>& svm, BlockCnt& bc){
		if (!bc.isloop){
			Error::GetInstance()->ProcessError("����:%d, ������ѭ�����в���ʹ�ùؼ���[continue]\n", token->GetLineNumber());
			
			return;
		}

		SVM::Instruction jump = { Opcode::JUMP, 0 };
		int cp = svm->AddCode(jump);

		bc.cps.push_back(cp);
	}
};

SABER_NAMESPACE_END

#endif