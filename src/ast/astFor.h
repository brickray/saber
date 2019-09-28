#ifndef H_AST_ASTFOR_H
#define H_AST_ASTFOR_H

#include "../astree.h"

SABER_NAMESPACE_BEGIN

class AstFor : public Astree{
private:
	bool generic = false;
public:
	void SetGeneric() { generic = true; }
	virtual void Compile(shared_ptr<Environment>& e, shared_ptr<SVM>& svm, BlockCnt& bc){
		SVM::Instruction nop(Opcode::NOP);

		BlockCnt subBc;
		subBc.isloop = true;
		subBc.cl = bc.cl;
		subBc.variableIndex = bc.variableIndex;
		subBc.maxLevel = bc.maxLevel;
		int end;
		int continueAddress;
		if (!generic){
			children[0]->Compile(e, svm, subBc);

			int loopAddress = svm->AddCode(nop);
			svm->RemoveLastCode();

			subBc.needRet = true;
			children[1]->Compile(e, svm, subBc);
			subBc.needRet = false;
			SVM::Instruction jz(Opcode::JZ);
			int jumpAddress = svm->AddCode(jz);

			children[children.size() - 1]->Compile(e, svm, subBc);

			continueAddress = svm->AddCode(nop);
			svm->RemoveLastCode();
			if (children.size() == 4){//has step component
				children[2]->Compile(e, svm, subBc);
			}

			SVM::Instruction jump(Opcode::JUMP, loopAddress);
			end = svm->AddCode(jump);
			jz.operand = end + 1;
			svm->SetCode(jumpAddress, jz);
		}
		else{
			int numChilds = GetNumChildren();
			subBc.needRet = true;
			children[numChilds - 2]->Compile(e, svm, subBc);
			subBc.needRet = false;
			int idx = svm->AddGlobal(Value());
			SVM::Instruction mov(Opcode::MOVE, idx);
			svm->AddCode(mov);

			int loopAddress = svm->AddCode(nop);
			continueAddress = loopAddress;	
			svm->RemoveLastCode();

			SVM::Instruction push(Opcode::PUSH, idx);
			SVM::Instruction call(Opcode::CALL, 0);
			call.operandb = true; //need return value
			svm->AddCode(push);
			svm->AddCode(call);

			for (int i = numChilds - 3; i >= 0; --i){
				children[i]->Compile(e, svm, subBc);
				mov.operand = subBc.nearst;
				mov.operands = subBc.nearstS;
				svm->AddCode(mov);
			}

			push.operand = subBc.nearst;
			push.operands = subBc.nearstS;
			svm->AddCode(push);
			SVM::Instruction jz(Opcode::JZ);
			int jumpAddress = svm->AddCode(jz);

			children[numChilds - 1]->Compile(e, svm, subBc);

			SVM::Instruction jump(Opcode::JUMP, loopAddress);
			end = svm->AddCode(jump);
			jz.operand = end + 1;
			svm->SetCode(jumpAddress, jz);
		}

		SVM::Instruction jumpb(Opcode::JUMP, end + 1);
		for (int i = 0; i < subBc.bps.size(); ++i){
			svm->SetCode(subBc.bps[i], jumpb);
		}
		SVM::Instruction jumpc(Opcode::JUMP, continueAddress);
		for (int i = 0; i < subBc.cps.size(); ++i){
			svm->SetCode(subBc.cps[i], jumpc);
		}

		bc.variableIndex = subBc.variableIndex;
	}
};

SABER_NAMESPACE_END

#endif