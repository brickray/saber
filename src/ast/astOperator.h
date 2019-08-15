#ifndef H_AST_ASTOPERATOR_H
#define H_AST_ASTOPERATOR_H

#include "../astree.h"

SABER_NAMESPACE_BEGIN

class AstOperator : public Astree{
public:
	virtual int Compile(shared_ptr<Environment>& e, shared_ptr<SVM>& svm, BlockCnt& bc){
		SVM::Instruction pop = { Opcode::POP };

		string op = token->GetToken();
		if (op == "="){
			Token* tok = children[0]->GetToken();
			if (tok->GetTokenType() != ETokenType::EIDENTIFIER){
				printf("行数:%d, 赋值语句非左值\n", tok->GetLineNumber());
				return -1;
			}


			int rightIdx = children[1]->Compile(e, svm, bc);
			
			int idx;
			if (e->HasSymbol(tok->GetToken())){
				idx = e->GetSymbol(tok->GetToken()).address;
			}
			else{
				Value v;
				idx = svm->AddGlobal(v);
				SymbolInfo si = { v, idx };
				e->SetSymbol(tok->GetToken(), si);
			}
			SVM::Instruction ins = { Opcode::MOVE, idx };
			svm->AddCode(ins);
			svm->AddCode(pop);

			return 0;
		}
		else if (op == "+"){
			int right = children[1]->Compile(e, svm, bc);
			int left = children[0]->Compile(e, svm, bc);

			SVM::Instruction ins = { Opcode::ADD };
			svm->AddCode(ins);
			svm->AddCode(pop);

			return 0;
		}
		else if (op == "-"){
			int right = children[1]->Compile(e, svm, bc);
			int left = children[0]->Compile(e, svm, bc);

			SVM::Instruction ins = { Opcode::SUB };
			svm->AddCode(ins);
			svm->AddCode(pop);

			return 0;
		}
		else if (op == "*"){
			int right = children[1]->Compile(e, svm, bc);
			int left = children[0]->Compile(e, svm, bc);

			SVM::Instruction ins = { Opcode::MUL };
			svm->AddCode(ins);
			svm->AddCode(pop);

			return 0;
		}
		else if (op == "/"){
			int right = children[1]->Compile(e, svm, bc);
			int left = children[0]->Compile(e, svm, bc);

			SVM::Instruction ins = { Opcode::DIV };
			svm->AddCode(ins);
			svm->AddCode(pop);

			return 0;
		}
		else if (op == "%"){
			int right = children[1]->Compile(e, svm, bc);
			int left = children[0]->Compile(e, svm, bc);

			SVM::Instruction ins = { Opcode::MOD };
			svm->AddCode(ins);
			svm->AddCode(pop);

			return 0;
		}
		else if (op == "+="){
			int right = children[1]->Compile(e, svm, bc);
			int left = children[0]->Compile(e, svm, bc);

			SVM::Instruction ins = { Opcode::ADD };
			svm->AddCode(ins);
			svm->AddCode(pop);
			ins.opcode = Opcode::MOVE;
			ins.operand = left;
			svm->AddCode(ins);

			return 0;
		}
		else if (op == "-="){
			int right = children[1]->Compile(e, svm, bc);
			int left = children[0]->Compile(e, svm, bc);
			int ret = left - right;

			SVM::Instruction ins = { Opcode::SUB };
			svm->AddCode(ins);
			svm->AddCode(pop);
			ins.opcode = Opcode::MOVE;
			ins.operand = left;
			svm->AddCode(ins);

			return 0;
		}
		else if (op == "*="){
			int right = children[1]->Compile(e, svm, bc);
			int left = children[0]->Compile(e, svm, bc);

			SVM::Instruction ins = { Opcode::MUL };
			svm->AddCode(ins);
			svm->AddCode(pop);
			ins.opcode = Opcode::MOVE;
			ins.operand = left;
			svm->AddCode(ins);

			return 0;
		}
		else if (op == "/="){
			int right = children[1]->Compile(e, svm, bc);
			int left = children[0]->Compile(e, svm, bc);
			int ret = left / right;

			SVM::Instruction ins = { Opcode::DIV };
			svm->AddCode(ins);
			svm->AddCode(pop);
			ins.opcode = Opcode::MOVE;
			ins.operand = left;
			svm->AddCode(ins);

			return 0;
		}
		else if (op == "%="){
			int right = children[1]->Compile(e, svm, bc);
			int left = children[0]->Compile(e, svm, bc);
			int ret = left % right;

			SVM::Instruction ins = { Opcode::MOD };
			svm->AddCode(ins);
			svm->AddCode(pop);
			ins.opcode = Opcode::MOVE;
			ins.operand = left;
			svm->AddCode(ins);

			return 0;
		}
		else if (op == "<"){
			int right = children[1]->Compile(e, svm, bc);
			int left = children[0]->Compile(e, svm, bc);

			SVM::Instruction ins = { Opcode::LT };
			svm->AddCode(ins);
			svm->AddCode(pop);

			return 0;
		}
		else if (op == ">"){
			int right = children[1]->Compile(e, svm, bc);
			int left = children[0]->Compile(e, svm, bc);

			SVM::Instruction ins = { Opcode::GT };
			svm->AddCode(ins);
			svm->AddCode(pop);

			return 0;
		}
		else if (op == "<="){
			int right = children[1]->Compile(e, svm, bc);
			int left = children[0]->Compile(e, svm, bc);
			
			SVM::Instruction ins = { Opcode::LE };
			svm->AddCode(ins);
			svm->AddCode(pop);

			return 0;
		}
		else if (op == ">="){
			int right = children[1]->Compile(e, svm, bc);
			int left = children[0]->Compile(e, svm, bc);
			
			SVM::Instruction ins = { Opcode::GE };
			svm->AddCode(ins);
			svm->AddCode(pop);

			return 0;
		}
		else if (op == "=="){
			int right = children[1]->Compile(e, svm, bc);
			int left = children[0]->Compile(e, svm, bc);
			
			SVM::Instruction ins = { Opcode::EQ };
			svm->AddCode(ins);
			svm->AddCode(pop);

			return 0;
		}
		else if (op == "!="){
			int right = children[1]->Compile(e, svm, bc);
			int left = children[0]->Compile(e, svm, bc);
			
			SVM::Instruction ins = { Opcode::NE };
			svm->AddCode(ins);
			svm->AddCode(pop);

			return 0;
		}
		else if (op == "||"){
			int right = children[1]->Compile(e, svm, bc);
			int left = children[0]->Compile(e, svm, bc);

			SVM::Instruction ins = { Opcode::OR };
			svm->AddCode(ins);
			svm->AddCode(pop);

			return 0;
		}
		else if (op == "&&"){
			int right = children[1]->Compile(e, svm, bc);
			int left = children[0]->Compile(e, svm, bc);

			SVM::Instruction ins = { Opcode::AND };
			svm->AddCode(ins);
			svm->AddCode(pop);

			return 0;
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

SABER_NAMESPACE_END

#endif