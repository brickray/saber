#ifndef H_AST_ASTWHILE_H
#define H_AST_ASTWHILE_H

#include "../astree.h"

class AstWhile : public Astree{
public:
	virtual int Eval(shared_ptr<Environment>& e, shared_ptr<VM>& vm){
		int loop = vm->AddCode(Opcode::NOP);
		int cond = children[0]->Eval(e, vm);

		int end = 0;
		vm->AddCode(Opcode::JZ);
		vm->AddCode(cond);
		int jump = vm->AddCode(end);

		for (int i = 1; i < children.size(); ++i)
			children[i]->Eval(e, vm);

		vm->AddCode(Opcode::JUMP);
		end = vm->AddCode(loop);
		vm->SetCode(jump, end + 1);

		return loop;
	}
};

#endif
