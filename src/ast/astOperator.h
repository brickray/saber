#ifndef H_AST_ASTOPERATOR_H
#define H_AST_ASTOPERATOR_H

#include "../astree.h"

SABER_NAMESPACE_BEGIN

class AstOperator : public Astree{
private:
	bool tbinit = false;
	int numVariables = 1;
public:
	void SetTableInit() { tbinit = true; }
	void SetNumVariables(int n) { numVariables = n; }
	virtual void Compile(shared_ptr<Environment>& e, shared_ptr<SVM>& svm, BlockCnt& bc){
		bc.needRet = true;

		string op = token->GetToken();
		if (op == "="){

			int childs = children.size();
			int loop = childs - numVariables > numVariables ? numVariables : childs - numVariables;
			for (int i = 0; i < loop; ++i){
				children[i + numVariables]->Compile(e, svm, bc);
				children[i]->Compile(e, svm, bc);

				if (istable & (1 << i)){
					SVM::Instruction ins(Opcode::STFILED);
					if (tbinit) ins.operand = 1;
					svm->AddCode(ins);

					SVM::Instruction dot(Opcode::GTFILED, 1);
					if (!tbinit)
						svm->SetCode(bc.nearst, dot);
				}
				else{
					SVM::Instruction ins(Opcode::MOVE, bc.nearst, bc.nearstS);
					svm->AddCode(ins);
				}
			}

			//不足用null补齐
			for (int i = numVariables - loop - 1; i >= 0; --i){
				children[loop + i]->Compile(e, svm, bc);
				SVM::Instruction pushn(Opcode::PUSHN);
				svm->AddCode(pushn);
				SVM::Instruction ins(Opcode::MOVE, bc.nearst, bc.nearstS);
				svm->AddCode(ins);
			}
		}
		else if (op == "+"){
			children[1]->Compile(e, svm, bc);
			children[0]->Compile(e, svm, bc);

			SVM::Instruction ins(Opcode::ADD);
			svm->AddCode(ins);
		}
		else if (op == "-"){
			if (children.size() == 1){
				//neg
				children[0]->Compile(e, svm, bc);
				SVM::Instruction ins(Opcode::NEG);
				svm->AddCode(ins);
			}
			else{ //sub
				children[1]->Compile(e, svm, bc);
				children[0]->Compile(e, svm, bc);

				SVM::Instruction ins(Opcode::SUB);
				svm->AddCode(ins);
			}
		}
		else if (op == "*"){
			children[1]->Compile(e, svm, bc);
			children[0]->Compile(e, svm, bc);

			SVM::Instruction ins(Opcode::MUL);
			svm->AddCode(ins);
		}
		else if (op == "/"){
			children[1]->Compile(e, svm, bc);
			children[0]->Compile(e, svm, bc);

			SVM::Instruction ins(Opcode::DIV);
			svm->AddCode(ins);
		}
		else if (op == "%"){
			children[1]->Compile(e, svm, bc);
			children[0]->Compile(e, svm, bc);

			SVM::Instruction ins(Opcode::MOD);
			svm->AddCode(ins);
		}
		else if (op == "+="){
			children[1]->Compile(e, svm, bc);
			children[0]->Compile(e, svm, bc);

			SVM::Instruction ins(Opcode::PLUSEQ);
			ins.operand = bc.nearst;
			ins.operands = bc.nearstS;
			svm->AddCode(ins);
		}
		else if (op == "-="){
			children[1]->Compile(e, svm, bc);
			children[0]->Compile(e, svm, bc);

			SVM::Instruction ins(Opcode::MIMUSEQ);
			ins.operand = bc.nearst;
			ins.operands = bc.nearstS;
			svm->AddCode(ins);
		}
		else if (op == "*="){
			children[1]->Compile(e, svm, bc);
			children[0]->Compile(e, svm, bc);

			SVM::Instruction ins(Opcode::MULEQ);
			ins.operand = bc.nearst;
			ins.operands = bc.nearstS;
			svm->AddCode(ins);
		}
		else if (op == "/="){
			children[1]->Compile(e, svm, bc);
			children[0]->Compile(e, svm, bc);

			SVM::Instruction ins(Opcode::DIVEQ);
			ins.operand = bc.nearst;
			ins.operands = bc.nearstS;
			svm->AddCode(ins);
		}
		else if (op == "%="){
			children[1]->Compile(e, svm, bc);
			children[0]->Compile(e, svm, bc);

			SVM::Instruction ins(Opcode::MODEQ);
			ins.operand = bc.nearst;
			ins.operands = bc.nearstS;
			svm->AddCode(ins);
		}
		else if (op == "<"){
			children[1]->Compile(e, svm, bc);
			children[0]->Compile(e, svm, bc);

			SVM::Instruction ins(Opcode::LT);
			svm->AddCode(ins);
		}
		else if (op == ">"){
			children[1]->Compile(e, svm, bc);
			children[0]->Compile(e, svm, bc);

			SVM::Instruction ins(Opcode::GT);
			svm->AddCode(ins);
		}
		else if (op == "<="){
			children[1]->Compile(e, svm, bc);
			children[0]->Compile(e, svm, bc);
			
			SVM::Instruction ins(Opcode::LE);
			svm->AddCode(ins);
		}
		else if (op == ">="){
			children[1]->Compile(e, svm, bc);
			children[0]->Compile(e, svm, bc);
			
			SVM::Instruction ins(Opcode::GE);
			svm->AddCode(ins);
		}
		else if (op == "=="){
			children[1]->Compile(e, svm, bc);
			children[0]->Compile(e, svm, bc);
			
			SVM::Instruction ins(Opcode::EQ);
			svm->AddCode(ins);
		}
		else if (op == "!="){
			children[1]->Compile(e, svm, bc);
			children[0]->Compile(e, svm, bc);
			
			SVM::Instruction ins(Opcode::NE);
			svm->AddCode(ins);
		}
		else if (op == "||"){
			children[0]->Compile(e, svm, bc);

			//或运算符左边为false则跳转到运算符右边的指令地址
			SVM::Instruction jz(Opcode::JZ);
			int jzAddress = svm->AddCode(jz);
			//或运算符左边为true则往栈里压入true然后跳过运算符右边的指令
			//因为不需要执行右边结果就已经确定
			SVM::Instruction push(Opcode::PUSHB, true);
			svm->AddCode(push);
			SVM::Instruction jump(Opcode::JUMP);
			int jumpAddress = svm->AddCode(jump);
			
			//左边为false则对右边进行求值
			jz.operand = jumpAddress + 1;
			svm->SetCode(jzAddress, jz);
			children[1]->Compile(e, svm, bc);

			SVM::Instruction ins(Opcode::OR);
			int next = svm->AddCode(ins) + 1;
			jump.operand = next;
			svm->SetCode(jumpAddress, jump);
		}
		else if (op == "&&"){
			children[0]->Compile(e, svm, bc);

			//与运算符左边为true则跳转到运算符右边的指令地址
			SVM::Instruction jnz(Opcode::JNZ);
			int jnzAddress = svm->AddCode(jnz);
			//与运算符左边为false则往栈里压入false然后跳过运算符右边的指令
			//执行结果必定为false
			//压入false然后跳转
			SVM::Instruction push(Opcode::PUSHB, false);
			svm->AddCode(push);
			SVM::Instruction jump(Opcode::JUMP);
			int jumpAddress = svm->AddCode(jump);

			//左边为true则对右边进行求值
			jnz.operand = jumpAddress + 1;
			svm->SetCode(jnzAddress, jnz);
			children[1]->Compile(e, svm, bc);

			SVM::Instruction ins(Opcode::AND);
			int next = svm->AddCode(ins) + 1;
			jump.operand = next;
			svm->SetCode(jumpAddress, jump);
		}
		else if (op == "!"){
			children[0]->Compile(e, svm, bc);
			SVM::Instruction ins(Opcode::NOT);
			svm->AddCode(ins);
		}

		bc.anyOperator = true;
		bc.lasttail = false;

		bc.needRet = false;
	}

	virtual string ToString() const{
		string ret;
		ret += children[0]->ToString();
		if (token){
			ret += token->GetToken();
			ret += " ";
		}
		ret += children[1]->ToString();

		return ret;
	}
};

SABER_NAMESPACE_END

#endif