#include "svm.h"
#include "opcode.h"
#include "error.h"

SABER_NAMESPACE_BEGIN

SVM::SVM(SState* s){
	S = s;

	code.reserve(64);
	stack.resize(STACK_SIZE);
	global.reserve(64);
	constant.reserve(64);
	ip = 0;
	sp = 0;
	cp = 0;
	offset = 0;
	ap = 0;
	fp = 0;
}

int SVM::AddCode(Instruction c){
	code.push_back(c);

	return code.size() - 1;
}

void SVM::RemoveLastCode(){
	code.pop_back();
}

SVM::Instruction SVM::GetLastCode(){
	return code[code.size() - 1];
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

void SVM::SetStack(int i, Value v){
	stack[i] = v;
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

void SVM::PushCoroutine(Coroutine* co){
	Value v;
	v.SetCoroutine(co);
	PushStack(v);
}

Value SVM::PopStack(){
	return stack[--sp];
}

void SVM::PushCo(Coroutine* c) {
	co.push_back(c);
}

Coroutine* SVM::PopCo() {
	if (co.size() == 0)
		return nullptr;
	Coroutine* c = co[co.size() - 1]; 
	co.pop_back();
	return c;
}

void SVM::Run(){
	//程序末尾加入结束命令
	SVM::Instruction exit(Opcode::EXIT);
	AddCode(exit);
	
	while (!IsEnd()){
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
		int p = func.GetFunction();
		int nfp = (p & 0x7f000000) >> 24;
		bool variable = (p & 0x80000000) >> 31;
		int nap = numParams;
		if ((!variable) && (nfp != numParams)){
			Error::GetInstance()->ProcessError("形参和实参数量不匹配");
		}

		int ncp = sp - nap;
		int nextip = ip + 1;
		stack[sp++].SetInt(nextip);
		stack[sp++].SetInt(ncp);
		stack[sp++].SetInt(cp);
		sp++; //variable parameters table
		stack[sp++].SetInt(offset);
		stack[sp++].SetInt(ap);
		stack[sp++].SetInt(fp);

		fp = nfp;
		ap = nap;
		offset = ap - fp;
		cp = ncp;
		if (variable){
			Table* t = new Table();
			stack[sp - 4].SetTable(int(t));
			//construct ...
			constructTDot(t, fp, ap);
		}

		ip = p & 0x00ffffff;

		while (true){
			execute();
			if (ip == nextip){
				ip--;
				break;
			}
			if (IsEnd()) break;
		}
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
	float operandf = ins.operandf;
	switch (op){
	case Opcode::MOVE:
		if (isStack(operand)){
			int o = cp + operand + ((operand >= fp + 3) ? offset : 0);
			stack[o] = stack[sp - 1];
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
			int p = func.GetFunction();
			int nfp = (p & 0x7f000000) >> 24;
			bool variable = (p & 0x80000000) >> 31;
			int nap = operand;
			if ((!variable) && (nfp != operand)){
				Error::GetInstance()->ProcessError("形参和实参数量不匹配");
			}

			int ncp = sp - nap;
			stack[sp++].SetInt(ip + 1);
			stack[sp++].SetInt(ncp);
			stack[sp++].SetInt(cp);
			sp++; //variable parameters table
			stack[sp++].SetInt(offset);
			stack[sp++].SetInt(ap);
			stack[sp++].SetInt(fp);

			fp = nfp;
			ap = nap;
			offset = ap - fp;
			cp = ncp;
			if (variable){
				Table* t = new Table();
				stack[sp - 4].SetTable(int(t));
				//construct ...
				constructTDot(t, fp, ap);
			}

			ip = p & 0x00ffffff;
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
		int base = cp + ap;
		int ofp     = stack[base + 6].GetInteger();
		int oap     = stack[base + 5].GetInteger();
		int ooffset = stack[base + 4].GetInteger();
		int tb      = stack[base + 3].GetInteger();
		int ocp     = stack[base + 2].GetInteger();
		int esp     = stack[base + 1].GetInteger();
		int eip     = stack[base + 0].GetInteger();

		//可变参函数
		if (ap != fp){
			Table* t = reinterpret_cast<Table*>(tb);
			delete t;
		}

		fp     = ofp;
		ap     = oap;
		offset = ooffset;
		cp     = ocp;
		sp     = esp;
		ip     = eip;
		if (numRetVariable) stack[sp++] = ret;

		return;
	}
	case Opcode::PUSHB:{
		stack[sp++].SetBool(operand);
		break;
	}
	case Opcode::PUSHI:{
		stack[sp++].SetInt(operand);
		break;
	}
	case Opcode::PUSHF:{
		stack[sp++].SetFloat(operandf);
		break;
	}
	case Opcode::PUSH:{
		Value src;

		if (relative){
			src = stack[sp - 1 + operand];
		}
		else{
			if (isConstant(operand)){
				src = constant[decodeConstantIndex(operand)];
			}
			else{
				int level = (operand & 0xff000000) >> 24;
				int idx = operand & 0x00ffffff;
				if (isStack(idx)){
					if (false){
						Table* table = reinterpret_cast<Table*>(stack[cp + ap + 3].GetTable());
						//find prev table
						for (int i = 0; i < level; ++i){
							table = reinterpret_cast<Table*>(table->kv["prev"].GetTable());
						}
						int o = idx + ((idx >= fp + 3) ? offset : 0);

						int i = table->kv["cv" + to_string(o)].GetInteger();
						src = stack[i];
					}
					else{
						int o = cp + idx + ((idx >= fp + 3) ? offset : 0);
						src = stack[o];
					}
				}
				else{//global
					src = global[decodeGlobalIndex(idx)];
				}
			}
		}

		stack[sp++] = src;
		break;
	}
	case Opcode::POP:{
		sp--;
		break;
	}
	case Opcode::RESERVE:{
		sp += operand;
		break;
	}
	case Opcode::GTFILED:{
		Value key = stack[sp - 1];
		Value table = stack[sp - 2];
		if (!table.IsTable()){
			Error::GetInstance()->ProcessError("尝试对非Table对象使用[.]");
		}
		if (!key.IsString() && !key.IsInteger()){
			Error::GetInstance()->ProcessError("key必须为integer或string");
		}

		Table* t = reinterpret_cast<Table*>(table.GetTable());
		string s = key.GetString();
		if (key.IsInteger()) s = to_string(key.GetInteger());
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
		if (!key.IsString() && !key.IsInteger()){
			Error::GetInstance()->ProcessError("key必须为integer或string");
		}
		Table* t = reinterpret_cast<Table*>(table.GetTable());
		string s = key.GetString();
		if (key.IsInteger()) s = to_string(key.GetInteger());
		t->kv[s] = value;

		sp -= 3;
		break;
	}
	case Opcode::EXIT:
		ip = code.size();
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
		stack[sp - 2] = stack[sp - 2] || stack[sp - 1];

		sp--;
		break;
	}
	case Opcode::AND:{
		stack[sp - 2] = stack[sp - 2] && stack[sp - 1];

		sp--;
		break;
	}
	case Opcode::NOP: //do nothing
		break;
	}

	ip++;
}

void SVM::constructTDot(Table* t, int fp, int ap){
	Value num;
	num.SetInt(ap - fp + 1);
	t->kv["num"] = num;
	for (int i = fp - 1; i < ap; ++i){
		Value p = stack[cp + i];
		t->kv[to_string(i - fp + 1)] = p;
	}
}

bool SVM::isStack(int idx){
	return idx >= 0 && idx < STACK_SIZE;
}

bool SVM::isGlobal(int idx){
	return idx >= STACK_SIZE;
}

bool SVM::isConstant(int idx){
	return idx < 0;
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
		"EXIT",
		"NOP",
		"GTFILED",
		"MOVE",
		"JZ",
		"JUMP",
		"CALL",
		"RET",
		"RESERVE",
		"PUSHB",
		"PUSHI",
		"PUSHF",
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
			if (c == "PUSHF"){
				ret += to_string(code[i].operandf);
			}
			else{
				ret += to_string(code[i].operand);
			}
		}
		else if (code[i].opcode >= Opcode::ENUM1 && code[i].opcode < Opcode::ENUM2){
			ret += "  ";
			if (c == "PUSH"){
				if (isConstant(code[i].operand)){
					ret += "\"" + constant[decodeConstantIndex(code[i].operand)].GetString() + "\"";
				}
				else{
					ret += to_string(code[i].operand);
				}
			}
			else{
				ret += to_string(code[i].operand);
			}
			ret += "  ";
			ret += to_string(code[i].relative);
		}

		ret += "\n";
	}

	return ret;
}

SABER_NAMESPACE_END