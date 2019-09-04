#include "svm.h"
#include "opcode.h"
#include "error.h"
#include "nativeFunc.h"

SABER_NAMESPACE_BEGIN

SVM::SVM(SState* s){
	S = s;

	code.reserve(64);
	stack.resize(STACK_SIZE);
	global.reserve(64);
	ip = 0;
	sp = 0;
	cp = 0;
	offset = 0;
	ap = 0;
	fp = 0;
	cl = nullptr;
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

void SVM::PushFunc(Closure* cl){
	Value v;
	v.SetFunction(cl);
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
		Closure* curCl = func.GetFunction();
		int p = curCl->entry;
		int nfp = (p & 0x7f000000) >> 24;
		bool variable = p & 0x80000000;
		int nap = numParams;
		if ((!variable) && (nfp != numParams)){
			Error::GetInstance()->ProcessError("形参和实参数量不匹配");
		}

		int ncp = sp - nap;
		int nextip = ip + 1;
		stack[sp++].SetInt(nextip);
		stack[sp++].SetInt(ncp);
		stack[sp++].SetInt(cp);
		stack[sp++].SetInt(offset);
		stack[sp++].SetInt(ap);
		stack[sp++].SetInt(fp);
		stack[sp++].SetFunction(cl);
		sp++; //variable parameters table

		fp = nfp;
		ap = nap;
		offset = ap - fp;
		cp = ncp;
		if (variable){
			Table* t = new Table();
			stack[sp - 1].SetTable(int(t));
			//construct ...
			constructTDot(t, fp, ap);
		}

		if (cl){
			Closure::VariableIterator vit = cl->ocvs.begin();
			for (; vit != cl->ocvs.end(); ++vit){
				curCl->ocvs[vit->first] = vit->second;
			}
		}

		cl = curCl;
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
	case Opcode::MOVE:{
		bool closure = operand & 0x40000000;
		int idx = operand & 0x3fffffff;
		if (closure){
			if (cl->ocvs.find(ins.operands) != cl->ocvs.end()){
				cl->ocvs[ins.operands] = stack[sp - 1];
			}
			else{
				idx = cl->cvs[idx];
				setClosureValue(idx, stack[sp - 1]);
			}
		}
		else{
			if (isStack(operand)){
				int o = cp + operand + ((operand >= fp + 3) ? offset : 0);
				stack[o] = stack[sp - 1];
			}
			else{
				global[decodeGlobalIndex(operand)] = stack[sp - 1];
			}
		}

		sp--;
		break;
	}
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
			Closure* curCl = func.GetFunction();
			int p = curCl->entry;
			int nfp = (p & 0x7f000000) >> 24;
			bool variable = p & 0x80000000;
			int nap = operand;
			if ((!variable) && (nfp != operand)){
				Error::GetInstance()->ProcessError("形参和实参数量不匹配");
			}

			int ncp = sp - nap;
			stack[sp++].SetInt(ip + 1);
			stack[sp++].SetInt(ncp);
			stack[sp++].SetInt(cp);
			stack[sp++].SetInt(offset);
			stack[sp++].SetInt(ap);
			stack[sp++].SetInt(fp);
			stack[sp++].SetFunction(cl);
			sp++; //variable parameters table

			fp = nfp;
			ap = nap;
			offset = ap - fp;
			cp = ncp;
			if (variable){
				Table* t = new Table();
				stack[sp - 1].SetTable(int(t));
				//construct ...
				constructTDot(t, fp, ap);
			}

			if (cl){
				Closure::VariableIterator vit = cl->ocvs.begin();
				for (; vit != cl->ocvs.end(); ++vit){
					curCl->ocvs[vit->first] = vit->second;
				}
			}

			cl = curCl;
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
		int tb       = stack[base + 7].GetTable();
		Closure* ocl = stack[base + 6].GetFunction();
		int ofp      = stack[base + 5].GetInteger();
		int oap      = stack[base + 4].GetInteger();
		int ooffset  = stack[base + 3].GetInteger();
		int ocp      = stack[base + 2].GetInteger();
		int esp      = stack[base + 1].GetInteger();
		int eip      = stack[base + 0].GetInteger();
		bool isCoroutine = eip & 0x80000000;
		eip = eip & 0x7fffffff;

		//可变参函数
		if (ap != fp){
			Table* t = reinterpret_cast<Table*>(tb);
			delete t;
		}

		if (ocl){
			Closure::VariableIterator vit = ocl->ocvs.begin();
			for (; vit != ocl->ocvs.end(); ++vit){
				ocl->ocvs[vit->first] = cl->ocvs[vit->first];
			}
		}
		//set closure value
		if (ret.IsFunction() && (ret.GetFunction() != cl)){
			Closure* o = ret.GetFunction();
			Closure* f = new Closure();
			f->entry = o->entry;
			f->variables = o->variables;
			Closure::VariableIterator vit = cl->ocvs.begin();
			for (; vit != cl->ocvs.end(); ++vit){
				f->ocvs[vit->first] = vit->second;
			}
			vit = cl->variables.begin();
			for (; vit != cl->variables.end(); ++vit){
				Value v;
				int idx = vit->second.GetInteger();
				if (isStack(idx)){
					int o = cp + idx + ((idx >= fp + 3) ? offset : 0);
					v = stack[o];
				}
				else{
					v = global[decodeGlobalIndex(idx)];
				}

				f->ocvs[vit->first] = v;
			}

			ret.SetFunction(f);
		}

		cl     = ocl;
		fp     = ofp;
		ap     = oap;
		offset = ooffset;
		cp     = ocp;
		sp     = esp;
		ip     = eip;
		if (numRetVariable) stack[sp++] = ret;

		if (isCoroutine) cocallback(this);

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
	case Opcode::PUSHS:{
		stack[sp++].SetString(ins.operands);
		break;
	}
	case Opcode::PUSH:{
		Value src;

		if (relative){
			src = stack[sp - 1 + operand];
		}
		else{
			bool closure = operand & 0x40000000;
			int idx = operand & 0x3fffffff;
			if (closure){
				if (cl->ocvs.find(ins.operands) != cl->ocvs.end()){
					src = cl->ocvs[ins.operands];;
				}
				else{
					idx = cl->cvs[idx];
					src = getClosureValue(idx);
				}
			}
			else{
				if (isStack(idx)){
					int o = cp + idx + ((idx >= fp + 3) ? offset : 0);
					src = stack[o];
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

Value SVM::getClosureValue(int op){
	int level = (op & 0xff000000) >> 24;
	int idx = op & 0x00ffffff;
	if (isStack(idx)){
		int tcp = cp;
		int tap = ap;
		int tfp = fp;
		int tof = offset;
		for (int i = 0; i < level; ++i){
			Value vcp = stack[tcp + tap + 2];
			Value vof = stack[tcp + tap + 4];
			Value vap = stack[tcp + tap + 5];
			Value vfp = stack[tcp + tap + 6];
			tcp = vcp.GetInteger();
			tof = vof.GetInteger();
			tap = vap.GetInteger();
			tfp = vfp.GetInteger();
		}

		int o = tcp + idx + ((idx >= tfp + 3) ? tof : 0);
		return stack[o];
	}
	else{
		return global[decodeGlobalIndex(idx)];
	}
}

void SVM::setClosureValue(int op, Value v){
	int level = (op & 0xff000000) >> 24;
	int idx = op & 0x00ffffff;
	if (isStack(idx)){
		int tcp = cp;
		int tap = ap;
		int tfp = fp;
		int tof = offset;
		for (int i = 0; i < level; ++i){
			Value vcp = stack[tcp + tap + 2];
			Value vof = stack[tcp + tap + 4];
			Value vap = stack[tcp + tap + 5];
			Value vfp = stack[tcp + tap + 6];
			tcp = vcp.GetInteger();
			tof = vof.GetInteger();
			tap = vap.GetInteger();
			tfp = vfp.GetInteger();
		}

		int o = tcp + idx + ((idx >= tfp + 3) ? tof : 0);
		stack[o] = v;
	}
	else{
		global[decodeGlobalIndex(idx)] = v;
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
		"PUSHS",
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
			else if (c == "PUSHS"){
				ret += code[i].operands;
			}
			else{
				ret += to_string(code[i].operand);
			}
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