#ifndef H_AST_ATSCONTINUE_H
#define H_AST_ASTCONTINUE_H

#include "../astree.h"

class AstContinue : public Astree{
public:
	virtual int Compile(shared_ptr<Environment>& e, shared_ptr<SVM>& svm, BlockCnt& bc){
		if (!bc.isloop){
			printf("����:%d, ������ѭ�����в���ʹ�ùؼ���[continue]\n", token->GetLineNumber());
			return 0;
		}

		SVM::Instruction jump = { Opcode::JUMP, bc.start };
		svm->AddCode(jump);

		return 0;
	}
};

#endif