#ifndef H_AST_ASTOPERATOR_H
#define H_AST_ASTOPERATOR_H

#include "../astree.h"

class AstOperator : public Astree{
public:
	virtual int Eval(shared_ptr<Environment>& e, shared_ptr<VM>& vm){
		string op = token->GetToken();
		if (op == "="){
			Token* tok = children[0]->GetToken();
			if (tok->GetTokenType() != ETokenType::EIDENTIFIER){
				printf("行数:%d, 赋值语句非左值\n", tok->GetLineNumber());
				return -1;
			}


			int rightIdx = children[1]->Eval(e, vm);
			
			vm->AddCode(Opcode::MOVE);
			int idx;
			if (e->HasSymbol(tok->GetToken())){
				idx = e->GetSymbol(tok->GetToken()).address;
			}
			else{
				Value v;
				idx = vm->AddStack(v);
				SymbolInfo si = { v, idx };
				e->SetSymbol(tok->GetToken(), si);
			}
			vm->AddCode(idx);
			vm->AddCode(rightIdx);

			return idx;
		}
		else if (op == "+"){
			int left = children[0]->Eval(e, vm);
			int right = children[1]->Eval(e, vm);

			int idx = vm->AddRegister(Value());
			vm->AddCode(Opcode::MOVE);
			vm->AddCode(idx);
			vm->AddCode(left);
			vm->AddCode(Opcode::ADD);
			vm->AddCode(idx);
			vm->AddCode(right);

			return idx;
		}
		else if (op == "-"){
			int left = children[0]->Eval(e, vm);
			int right = children[1]->Eval(e, vm);

			int idx = vm->AddRegister(Value());
			vm->AddCode(Opcode::MOVE);
			vm->AddCode(idx);
			vm->AddCode(left);
			vm->AddCode(Opcode::SUB);
			vm->AddCode(idx);
			vm->AddCode(right);

			return idx;
		}
		else if (op == "*"){
			int left = children[0]->Eval(e, vm);
			int right = children[1]->Eval(e, vm);

			int idx = vm->AddRegister(Value());
			vm->AddCode(Opcode::MOVE);
			vm->AddCode(idx);
			vm->AddCode(left);
			vm->AddCode(Opcode::MUL);
			vm->AddCode(idx);
			vm->AddCode(right);

			return idx;
		}
		else if (op == "/"){
			int left = children[0]->Eval(e, vm);
			int right = children[1]->Eval(e, vm);

			int idx = vm->AddRegister(Value());
			vm->AddCode(Opcode::MOVE);
			vm->AddCode(idx);
			vm->AddCode(left); 
			vm->AddCode(Opcode::DIV);
			vm->AddCode(idx);
			vm->AddCode(right);

			return idx;
		}
		else if (op == "%"){
			int left = children[0]->Eval(e, vm);
			int right = children[1]->Eval(e, vm);

			int idx = vm->AddRegister(Value());
			vm->AddCode(Opcode::MOVE);
			vm->AddCode(idx);
			vm->AddCode(left); 
			vm->AddCode(Opcode::MOD);
			vm->AddCode(idx);
			vm->AddCode(right);

			return idx;
		}
		else if (op == "+="){
			int left = children[0]->Eval(e, vm);
			int right = children[1]->Eval(e, vm);

			vm->AddCode(Opcode::ADD);
			vm->AddCode(left);
			vm->AddCode(right);

			return left;
		}
		else if (op == "-="){
			int left = children[0]->Eval(e, vm);
			int right = children[1]->Eval(e, vm);
			int ret = left - right;

			vm->AddCode(Opcode::SUB);
			vm->AddCode(left);
			vm->AddCode(right);

			return left;
		}
		else if (op == "*="){
			int left = children[0]->Eval(e, vm);
			int right = children[1]->Eval(e, vm);
			int ret = left * right;

			vm->AddCode(Opcode::MUL);
			vm->AddCode(left);
			vm->AddCode(right);

			return left;
		}
		else if (op == "/="){
			int left = children[0]->Eval(e, vm);
			int right = children[1]->Eval(e, vm);
			int ret = left / right;

			vm->AddCode(Opcode::DIV);
			vm->AddCode(left);
			vm->AddCode(right);

			return left;
		}
		else if (op == "%="){
			int left = children[0]->Eval(e, vm);
			int right = children[1]->Eval(e, vm);
			int ret = left % right;

			vm->AddCode(Opcode::MOD);
			vm->AddCode(left);
			vm->AddCode(right);

			return left;
		}
		else if (op == "<"){
			int left = children[0]->Eval(e, vm);
			int right = children[1]->Eval(e, vm);

			int idx = vm->AddRegister(Value());
			vm->AddCode(Opcode::GT);
			vm->AddCode(idx);
			vm->AddCode(right);
			vm->AddCode(left);

			return idx;
		}
		else if (op == ">"){
			int left = children[0]->Eval(e, vm);
			int right = children[1]->Eval(e, vm);
			int idx = vm->AddRegister(Value());
			vm->AddCode(Opcode::GT);
			vm->AddCode(idx);
			vm->AddCode(left);
			vm->AddCode(right);

			return idx;
		}
		else if (op == "<="){
			int left = children[0]->Eval(e, vm);
			int right = children[1]->Eval(e, vm);
			int idx = vm->AddRegister(Value());
			vm->AddCode(Opcode::GE);
			vm->AddCode(idx);
			vm->AddCode(right);
			vm->AddCode(left);

			return idx;
		}
		else if (op == ">="){
			int left = children[0]->Eval(e, vm);
			int right = children[1]->Eval(e, vm);
			int idx = vm->AddRegister(Value());
			vm->AddCode(Opcode::GE);
			vm->AddCode(idx);
			vm->AddCode(left);
			vm->AddCode(right);

			return idx;
		}
		else if (op == "=="){
			int left = children[0]->Eval(e, vm);
			int right = children[1]->Eval(e, vm);
			int idx = vm->AddRegister(Value());
			vm->AddCode(Opcode::EQ);
			vm->AddCode(idx);
			vm->AddCode(left);
			vm->AddCode(right);

			return idx;
		}
		else if (op == "!="){
			int left = children[0]->Eval(e, vm);
			int right = children[1]->Eval(e, vm);
			int idx = vm->AddRegister(Value());
			vm->AddCode(Opcode::NE);
			vm->AddCode(idx);
			vm->AddCode(left);
			vm->AddCode(right);

			return idx;
		}
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

#endif