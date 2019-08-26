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
	cp = 0;

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

void SVM::PushFunc(int i){
	Value v;
	v.SetFunction(i);
	PushStack(v);
}

void SVM::PushNativeFunc(SFunc f){
	Value v;
	v.SetNativeFunction(f);
	PushStack(v);
}

void SVM::PushLightUData(int i){
	Value v;
	v.SetLightUData(i);
	PushStack(v);
}

void SVM::PushTable(int i){
	Value v;
	v.SetTable(i);
	PushStack(v);
}

Value SVM::PopStack(){
	return stack[--sp];
}

void SVM::Run(){
	int numCode = code.size();
	while (ip < numCode){
		execute();
	}
}

void SVM::CallScript(int numParams){
	Value func = stack[sp - 1];
	sp--; //pop
	if (func.IsNativeFunction()){
		func.GetNativeFunction()(this, numParams);
	}
	else if (func.IsFunction()){
		Value eip, esp;
		int ncp = sp - numParams;
		eip.SetInt(ip + 1);
		stack[sp++] = eip;
		esp.SetInt(ncp);
		stack[sp++] = esp;
		stack[sp++].SetInt(cp);
		cp = ncp;

		ip = func.GetFunction();

		execute();
	}
	else{
		Error::GetInstance()->ProcessError("尝试对[%s]值进行函数调用\n", func.GetTypeString().c_str());
	}
}

void SVM::execute(){
	Instruction ins = code[ip];
	char op = ins.opcode;
	bool relative = ins.relative;
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
			return;
		}

		break;
	}
	case Opcode::JUMP:
		ip = operand;

		return;
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
			return;
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

		return;
	}
	case Opcode::PUSH:{
		Value src;
		if (relative){
			src = stack[sp - 1 + operand];
		}
		else{
			if (isStack(operand)) src = stack[cp + operand];
			else if (isGlobal(operand)) src = global[decodeGlobalIndex(operand)];
			else src = constant[decodeConstantIndex(operand)];
		}

		stack[sp++] = src;
		break;
	}
	case Opcode::POP:
		sp--;
		break;
	case Opcode::RESERVE:
		sp += operand;
		break;
	case Opcode::GTFILED:{
		Value key = stack[sp - 1];
		Value table = stack[sp - 2];
		if (!table.IsTable()){
			Error::GetInstance()->ProcessError("尝试对非Table对象使用[.]");
		}
		if (!key.IsString()){
			Error::GetInstance()->ProcessError("key必须为string");
		}

		Table* t = reinterpret_cast<Table*>(table.GetTable());
		string s = key.GetString();
		Value value;
		if (t->kv.find(s) != t->kv.end()){
			value = t->kv[s];
		}

		if (!operand){
			stack[sp - 2] = value;
			sp--;
		}

		break;
	}
	case Opcode::SETTABLE:{
		Table* t = new Table();
		stack[sp++].SetTable((int)t);

		break;
	}
	case Opcode::STFILED:{
		Value key = stack[sp - 1];
		Value table = stack[sp - 2];
		Value value = stack[sp - 3];
		if (!table.IsTable()){
			Error::GetInstance()->ProcessError("尝试对非Table对象使用[.]");
		}
		if (!key.IsString()){
			Error::GetInstance()->ProcessError("key必须为string");
		}
		Table* t = reinterpret_cast<Table*>(table.GetTable());
		t->kv[key.GetString()] = value;

		sp -= 3;
		break;
	}
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
		"SETTABLE",
		"STFILED",
		"NOP",
		"GTFILED",
		"MOVE",
		"JZ",
		"JUMP",
		"CALL",
		"RET",
		"RESERVE",
		"PUSH",
	};

	string ret;
	for (int i = 0; i < code.size(); ++i){
		string c = codeString[code[i].opcode];
		ret += to_string(i);
		ret += "  ";
		ret += c;
		if (code[i].opcode >= Opcode::ENUM0 && code[i].opcode < Opcode::ENUM1){
			ret += "  ";
			ret += to_string(code[i].operand);
		}
		else if (code[i].opcode >= Opcode::ENUM1 && code[i].opcode < Opcode::ENUM2){
			ret += "  ";
			ret += to_string(code[i].operand);
			ret += "  ";
			ret += to_string(code[i].relative);
		}

		ret += "\n";
	}

	return ret;
}

SABER_NAMESPACE_END