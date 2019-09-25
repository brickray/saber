#ifndef H_AST_ASTDOT_H
#define H_AST_ASTDOT_H

#include "../astree.h"

SABER_NAMESPACE_BEGIN

class AstDot : public Astree{
private:
	bool fromFunc = false;
public:
	void SetFunc(bool f) { fromFunc = f; }
	virtual void Compile(shared_ptr<Environment>& e, shared_ptr<SVM>& svm, BlockCnt& bc){
		AstDot* d = dynamic_cast<AstDot*>(children[0].get());
		if (bc.getad) bc.getad = false;
		children[0]->Compile(e, svm, bc);
		if (!fromFunc){
			children[1]->Compile(e, svm, bc);

			SVM::Instruction dot(Opcode::GTFILED);
			bc.nearst = svm->AddCode(dot);
		}
		else if (!d){
			SVM::Instruction dot(Opcode::GTFILED);
			bc.nearst = svm->AddCode(dot);

			if (children.size() > 1){
				AstDot* d = dynamic_cast<AstDot*>(children[1].get());
				children[1]->Compile(e, svm, bc);
				if (!d) bc.nearst = svm->AddCode(dot);
			}
		}
	}
};

SABER_NAMESPACE_END

#endif