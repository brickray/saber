#include "svm.h"
#include "opcode.h"
#include "error.h"

SABER_NAMESPACE_BEGIN

SVM::SVM(){
	code.reserve(64);
	stack.resize(STACK_SIZE);
	global.reserve(64);
	constant.reserve(64);
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

void SVM::RemoveLastCode(){
	code.pop_back();
}

SVM::Instruction SVM::GetCode(int idx){
	return code[idx];
}

void SVM::SetCode(int idx, Instruction c){
	code[idx] = c;
}

int SVM::AddGlobal(Value v){
	global.push_back(v);
	
	return encodeGlobalIndex(global.size() - 1);
}

int SVM::AddConstant(Value v){
	constant.push_back(v);

	return encodeConstantIndex(constant.size() - 1);
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
		switch (op){
		case Opcode::MOVE:
			if (isStack(operand)){
				stack[cp + operand] = stack[sp - 1];
			}
			else{
				global[decodeGlobalIndex(operand)] = stack[sp - 1];
			}

			sp--;
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
				Value eip, esp;
				int ncp = sp - operand;
				eip.SetInt(ip + 1);
				stack[sp++] = eip;
				esp.SetInt(ncp);
				stack[sp++] = esp;
				stack[sp++].SetInt(cp);
				cp = ncp;

				ip = func.GetFunction();
				continue;
			}
			else{
				Error::GetInstance()->ProcessError("尝试对[%s]值进行函数调用\n", func.GetTypeString().c_str());
				break;
			}
		}
		case Opcode::RET:{
			Value ret = stack[sp - 1]; 
			int numRetVariable = (operand & 0xffff0000) >> 16;
			int numParams = operand & 0x0000ffff;
			int base = cp + numParams;
			int ocp = stack[base + 2].GetInteger();
			int esp = stack[base + 1].GetInteger();
			int eip = stack[base + 0].GetInteger();
			cp = ocp;
			sp = esp;
			ip = eip;
			if (numRetVariable) stack[sp++] = ret;

			continue;
		}
		case Opcode::PUSH:{
			Value src;
			if (isStack(operand)) src = stack[cp + operand];
			else if (isGlobal(operand)) src = global[decodeGlobalIndex(operand)];
			else src = constant[decodeConstantIndex(operand)];

			stack[sp++] = src;
			break;
		}
		case Opcode::POP:
			sp--;
			break;
		case Opcode::RESERVE:
			sp += operand;
			break;
		case Opcode::NEG:
			stack[sp - 1] = -stack[sp - 1];

			break;
		case Opcode::ADD:
			stack[sp - 2] = stack[sp - 1] + stack[sp - 2];

			sp--;
			break;
		case Opcode::SUB:
			stack[sp - 2] = stack[sp - 1] - stack[sp - 2];

			sp--;
			break;
		case Opcode::MUL:
			stack[sp - 2] = stack[sp - 1] * stack[sp - 2];

			sp--;
			break;
		case Opcode::DIV:
			stack[sp - 2] = stack[sp - 1] / stack[sp - 2];

			sp--;
			break;
		case Opcode::MOD:
			stack[sp - 2] = stack[sp - 1] % stack[sp - 2];

			sp--;
			break;
		case Opcode::GT:
			stack[sp - 2] = stack[sp - 2] < stack[sp - 1];

			sp--;
			break;
		case Opcode::LT:
			stack[sp - 2] = stack[sp - 2] > stack[sp - 1];

			sp--;
			break;
		case Opcode::GE:
			stack[sp - 2] = stack[sp - 2] <= stack[sp - 1];

			sp--;
			break;
		case Opcode::LE:
			stack[sp - 2] = stack[sp - 2] >= stack[sp - 1];

			sp--;
			break;
		case Opcode::EQ:
			stack[sp - 2] = stack[sp - 2] == stack[sp - 1];

			sp--;
			break;
		case Opcode::NE:
			stack[sp - 2] = stack[sp - 2] != stack[sp - 1];

			sp--;
			break;
		case Opcode::OR:{
			bool t1, t2;
			if (stack[sp - 1].IsFloat()) t1 = stack[sp - 1].GetFloat() != 0;
			else t1 = stack[sp - 1].GetBoolean();
			if (stack[sp - 2].IsFloat()) t2 = stack[sp - 2].GetFloat() != 0;
			else t2 = stack[sp - 2].GetBoolean();

			stack[sp - 2].SetBool(t1 || t2);

			sp--;
			break;
		}
		case Opcode::AND:{
			bool t1, t2;
			if (stack[sp - 1].IsFloat()) t1 = stack[sp - 1].GetFloat() != 0;
			else t1 = stack[sp - 1].GetBoolean();
			if (stack[sp - 2].IsFloat()) t2 = stack[sp - 2].GetFloat() != 0;
			else t2 = stack[sp - 2].GetBoolean();

			stack[sp - 2].SetBool(t1 && t2);

			sp--;
			break;
		}
		case Opcode::NOP: //do nothing
			break;
		}

		ip++;
	}
}

bool SVM::isStack(int idx){
	return idx >= 0 && idx < STACK_SIZE;
}

bool SVM::isGlobal(int idx){
	return idx >= STACK_SIZE;
}

int SVM::encodeGlobalIndex(int idx){
	return STACK_SIZE + idx;
}

int SVM::decodeGlobalIndex(int idx){
	return idx - STACK_SIZE;
}

int SVM::encodeConstantIndex(int idx){
	return -idx - 1;
}

int SVM::decodeConstantIndex(int idx){
	return -(idx + 1);
}

string SVM::ShowCode(){
	static string codeString[Opcode::ECNUM] = {
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
		"JZ",
		"JUMP",
		"CALL",
		"RET",
		"PUSH",
		"RESERVE",
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

SABER_NAMESPACE_END