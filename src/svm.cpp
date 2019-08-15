#include "svm.h"
#include "opcode.h"

SVM::SVM(){
	code.reserve(64);
	stack.resize(1024);
	registers.reserve(NUM_REGISTER);
	ip = 0;
	sp = 0;

	Value t, f;
	t.SetBool(true);
	f.SetBool(false);
	AddConstant(t);
	AddConstant(f);
}

int SVM::AddCode(Instruction c){
	code.push_back(c);

	return code.size() - 1;
}

void SVM::SetCode(int idx, Instruction c){
	code[idx] = c;
}

int SVM::AddGlobal(Value v){
	global.push_back(v);
	
	return global.size() - 1;
}

int SVM::AddConstant(Value v){
	constant.push_back(v);

	return encodeConstantIndex(constant.size());
}

void SVM::PushStack(Value v){
	stack[sp++] = v;
}

void SVM::PushBool(bool b){
	Value v;
	v.SetBool(b);
	PushStack(v);
}

void SVM::PushInt(int i){
	Value v;
	v.SetInt(i);
	PushStack(v);
}

void SVM::PushFloat(float f){
	Value v;
	v.SetFloat(f);
	PushStack(v);
}

void SVM::PushString(string s){
	Value v;
	v.SetString(s);
	PushStack(v);
}

Value SVM::PopStack(){
	return stack[--sp];
}

void SVM::Run(){
	int numCode = code.size();
	while (ip < numCode){
		Instruction ins = code[ip];
		char op = ins.opcode;
		int operand = ins.operand;
		int operand1 = ins.operand1;
		switch (op){
		case Opcode::MOVE:
			global[operand] = stack[sp - 1];

			break;
		case Opcode::JZ:{
			bool t;
			if (stack[sp - 1].IsFloat()) t = stack[sp - 1].GetFloat() != 0;
			else t = stack[sp - 1].GetBoolean();

			sp--;
			if (!t){
				ip = operand;
				continue;
			}

			break;
		}
		case Opcode::JUMP:
			ip = operand;

			continue;
		case Opcode::CALL:{
			Value func = stack[sp - 1];
			sp--; //pop
			if (func.IsNativeFunction()){
				func.GetNativeFunction()(this, operand);
				break;
			}
			else if (func.IsFunction()){
				ip = func.GetInteger();
				Value ret;
				ret.SetInt(operand1);
				stack[sp++] = ret;
				continue;
			}
			else{
				printf("对非函数进行调用\n");
				exit(1);
				break;
			}
		}
		case Opcode::RET:
			ip = stack[sp - 1].GetInteger();

			sp -= operand + 1;
			continue;
		case Opcode::PUSH:{
			Value src;
			if (operand1 == 0){
				if (isGlobal(operand)) src = global[operand];
				else src = constant[decodeConstantIndex(operand)];
			}
			else{
				src.SetInt(operand);
			}
			stack[sp++] = src;
			break;
		}
		case Opcode::POP:
			sp--;
			break;
		case Opcode::ADD:
			stack[sp - 2] = stack[sp - 1] + stack[sp - 2];

			break;
		case Opcode::SUB:
			stack[sp - 2] = stack[sp - 1] - stack[sp - 2];
			
			break;
		case Opcode::MUL:
			stack[sp - 2] = stack[sp - 1] * stack[sp - 2];
			
			break;
		case Opcode::DIV:
			stack[sp - 2] = stack[sp - 1] / stack[sp - 2];
			
			break;
		case Opcode::MOD:
			stack[sp - 2] = stack[sp - 1] % stack[sp - 2];
			
			break;
		case Opcode::GT:
			stack[sp - 2] = stack[sp - 2] < stack[sp - 1];
			
			break;
		case Opcode::LT:
			stack[sp - 2] = stack[sp - 2] > stack[sp - 1];
			
			break;
		case Opcode::GE:
			stack[sp - 2] = stack[sp - 2] <= stack[sp - 1];
			
			break;
		case Opcode::LE:
			stack[sp - 2] = stack[sp - 2] >= stack[sp - 1];
			
			break;
		case Opcode::EQ:
			stack[sp - 2] = stack[sp - 2] == stack[sp - 1];
			
			break;
		case Opcode::NE:
			stack[sp - 2] = stack[sp - 2] != stack[sp - 1];
			
			break;
		case Opcode::OR:{
			bool t1, t2;
			if (stack[sp - 1].IsFloat()) t1 = stack[sp - 1].GetFloat() != 0;
			else t1 = stack[sp - 1].GetBoolean();
			if (stack[sp - 2].IsFloat()) t2 = stack[sp - 2].GetFloat() != 0;
			else t2 = stack[sp - 2].GetBoolean();

			stack[sp - 2].SetBool(t1 || t2);

			break;
		}
		case Opcode::AND:{
			bool t1, t2;
			if (stack[sp - 1].IsFloat()) t1 = stack[sp - 1].GetFloat() != 0;
			else t1 = stack[sp - 1].GetBoolean();
			if (stack[sp - 2].IsFloat()) t2 = stack[sp - 2].GetFloat() != 0;
			else t2 = stack[sp - 2].GetBoolean();

			stack[sp - 2].SetBool(t1 && t2);

			break;
		}
		case Opcode::NOP: //do nothing
			break;
		}

		ip++;
	}
}

bool SVM::isGlobal(int idx){
	return idx >= 0;
}

int SVM::encodeConstantIndex(int idx){
	return -idx - NUM_REGISTER;
}

int SVM::decodeConstantIndex(int idx){
	return -(idx + NUM_REGISTER + 1);
}

string SVM::ShowCode(){
	static string codeString[Opcode::ECNUM] = {
		"JZ",
		"POP",
		"NEG",
		"ADD",
		"SUB",
		"MUL",
		"DIV",
		"MOD",
		"GT",
		"LT",
		"GE",
		"LE",
		"EQ",
		"NE",
		"OR",
		"AND",
		"NOP",
		"MOVE",
		"JUMP",
		"CALL",
		"RET",
		"PUSH",
	};

	string ret;
	for (int i = 0; i < code.size(); ++i){
		string c = codeString[code[i].opcode];
		ret += to_string(i);
		ret += "  ";
		ret += c;
		if (code[i].opcode >= Opcode::ENUM0){
			ret += "  ";
			ret += to_string(code[i].operand);
		}

		ret += "\n";
	}

	return ret;
}