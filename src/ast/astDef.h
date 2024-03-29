#ifndef H_AST_ASTDEF_H
#define H_AST_ASTDEF_H

#include "../astree.h"

SABER_NAMESPACE_BEGIN

class AstDef : public Astree{
public:
	virtual void Compile(shared_ptr<Environment>& e, shared_ptr<SVM>& svm, BlockCnt& bc){
		SVM::Instruction nop(Opcode::NOP);

		int numParams = children.size() - 2;
		int next = 0;
		SVM::Instruction jump(Opcode::JUMP, next);
		int jumpAddress = svm->AddCode(jump);
		int start = jumpAddress + 1;
		SVM::Instruction reserve(Opcode::RESERVE, 0);
		int reserveAddress = svm->AddCode(reserve);
		Value func;
		//set closure
		Clptr cl = Clptr(new Closure());
		cl->closure = false;
		cl->hascv = false;
		cl->vararg = false;
		cl->needRet = false;
		cl->entry = start;
		cl->fp = numParams;
		func.SetFunction(cl);
		int funcAddress = svm->AddGlobal(func);
		SymbolInfo si = { func, funcAddress };
		string funcName = children[0]->GetToken()->GetToken();
		e->SetSymbol(funcName, si);
		if (bc.cl){
			bc.cl->variables[funcName] = si.address;
			bc.cl->cls.insert(cl);
		}

		//set environment
		shared_ptr<Environment> local = shared_ptr<Environment>(new Environment());
		local->SetOutter(e);
		for (int i = 1; i < numParams + 1; ++i){
			string name = children[i]->GetToken()->GetToken();
			if (name == "..."){
				name = "args";
				SymbolInfo si;
				si.address = numParams + TB_ADDRESS;
				local->SetSymbol(name, si);

				cl->variables[name] = si.address;
				cl->vararg = true;
			}
			else{
				SymbolInfo si;
				si.address = i - 1;
				local->SetSymbol(name, si);

				cl->variables[name] = i - 1;
			}
		}

		BlockCnt subBc;
		subBc.cl = cl;
		subBc.maxLevel = bc.maxLevel + 1;
		int numSpace = NUM_ADDRESS;
		subBc.variableIndex = numParams + numSpace;
		for (int i = numParams + 1; i < children.size(); ++i){
			children[i]->Compile(local, svm, subBc);
		}
		reserve.operand = subBc.variableIndex - numParams - numSpace;
		if (!reserve.operand){
			//如果函数内的局部变量不存在，直接将reserve指令清除
			svm->SetCode(reserveAddress, nop);
		}
		else {
			svm->SetCode(reserveAddress, reserve);
		}

		if (subBc.lasttail){
			//函数末尾没return，且递归调用的情况为尾递归
			SVM::Instruction tail(Opcode::TAILCALL);
			svm->SetCode(subBc.nearst, tail);
		}

		//如果函数最后没有return则加上
		SVM::Instruction ret(Opcode::RET, 0);
		SVM::Instruction last = svm->GetLastCode();
		if (last.opcode != Opcode::RET){
			svm->AddCode(ret);
		}

		next = svm->AddCode(nop);
		svm->RemoveLastCode();
		jump.operand = next;
		svm->SetCode(jumpAddress, jump);
	}
};

SABER_NAMESPACE_END

#endif