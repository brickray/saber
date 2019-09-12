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

void SVM::PushFunc(Clptr cl){
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

void SVM::PushTable(Tptr t){
	Value v;
	v.SetTable(t);
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
		Clptr curCl = func.GetFunction();
		curCl->childs.clear();
		int p = curCl->entry;
		int nfp = (p & 0x7f000000) >> 24;
		bool variable = p & 0x80000000;
		bool outer = curCl->level <= (cl ? cl->level : 0);
		int nap = numParams;
		if ((!variable) && (nfp != nap)){
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
			Tptr t = shared_ptr<Table>(new Table());
			stack[sp - 1].SetTable(t);
			//construct ...
			constructTDot(t, fp, ap);
		}

		if (cl && !outer){
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
		move(operand, ins.operands);

		break;
	}
	case Opcode::JZ:{
		bool t;
		if (stack[sp - 1].IsNull()) t = false;
		else if (stack[sp - 1].IsBoolean()) t = stack[sp - 1].GetBoolean();
		else if (stack[sp - 1].IsFloat()) t = stack[sp - 1].GetFloat() != 0;
		else if (stack[sp - 1].IsInteger()) t = stack[sp - 1].GetInteger() != 0;
		else t = true;

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
			Clptr curCl = func.GetFunction();
			curCl->childs.clear();
			int p = curCl->entry;
			int nfp = (p & 0x7f000000) >> 24;
			bool variable = p & 0x80000000;
			bool outer = curCl->level <= (cl ? cl->level : 0);
			int nap = operand;
			if ((!variable) && (nfp != nap)){
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
				Tptr t = shared_ptr<Table>(new Table());
				stack[sp - 1].SetTable(t);
				//construct ...
				constructTDot(t, fp, ap);
			}

			if (cl && !outer){
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
		Tptr t       = stack[base + 7].GetTable();
		Clptr ocl    = stack[base + 6].GetFunction();
		int ofp      = stack[base + 5].GetInteger();
		int oap      = stack[base + 4].GetInteger();
		int ooffset  = stack[base + 3].GetInteger();
		int ocp      = stack[base + 2].GetInteger();
		int esp      = stack[base + 1].GetInteger();
		int eip      = stack[base + 0].GetInteger();
		bool isCoroutine = eip & 0x80000000;
		eip = eip & 0x7fffffff;

		if (ocl){
			Closure::VariableIterator vit = ocl->ocvs.begin();
			for (; vit != ocl->ocvs.end(); ++vit){
				ocl->ocvs[vit->first] = cl->ocvs[vit->first];
			}
		}
		//set closure value
		Closure::ClosureIterator it = cl->childs.begin();
		Closure::ClosureIterator ited = cl->childs.end();
		for (; it != ited; ++it) (*it)->SetFunction(createClosure((*it)->GetFunction()));
		
		if (numRetVariable && ret.IsFunction() && (ret.GetFunction() != cl)){
			ret.SetFunction(createClosure(ret.GetFunction()));
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
	case Opcode::GETLEN:{
		Value v = stack[sp - 1];
		if (v.IsTable()){
			stack[sp - 1].SetInt(v.GetTable()->GetLength());
		}
		else if (v.IsString()){
			stack[sp - 1].SetInt(v.GetString().size());
		}
		else{
			Error::GetInstance()->ProcessError("尝试对类型%s使用#", v.GetTypeString().c_str());
		}

		break;
	}
	case Opcode::PUSHN:{
		stack[sp++].SetNull();
		break;
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
			//这个位置必定是函数地址
			src = stack[sp - 1 + operand];
			//将函数引用计数减一
			stack[sp - 1 + operand].SetNull();
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

		Tptr t = table.GetTable();
		string s = key.GetString();
		if (key.IsInteger()) s = to_string(key.GetInteger());
		Value value;
		if (!t->HasValue(s)){
			if (!operand && t->HasValue("_default")){
				Value func = t->GetValue("_default");

				PushString(s);
				PushStack(func);
				CallScript(1);
			}
		}
		else{
			value = t->GetValue(s);
		}

		if (!operand){
			if (value.IsNativeFunction() && t->HasValue(SELF)){
				stack[sp - 1] = value;
			}
			else{
				stack[sp - 2] = value;
				sp--;
			}
		}

		break;
	}
	case Opcode::SETTABLE:{
		Tptr t = shared_ptr<Table>(new Table());
		stack[sp++].SetTable(t);

		break;
	}
	case Opcode::STFILED:{
		Value key = stack[sp - 1];
		Value table = stack[sp - 2];
		Value value = stack[sp - 3];
		if (operand){//table init
			key = stack[sp - 1];
			value = stack[sp - 2];
			table = stack[sp - 3];
		}

		if (!table.IsTable()){
			Error::GetInstance()->ProcessError("尝试对非Table对象使用[.]");
		}
		if (!key.IsString() && !key.IsInteger()){
			Error::GetInstance()->ProcessError("key必须为integer或string");
		}
		Tptr t = table.GetTable();
		string s = key.GetString();
		if (key.IsInteger()) s = to_string(key.GetInteger());
		t->AddValue(s, value);
		if (cl){
			Value* ptr = t->GetValuePtr(s);
			if (value.IsFunction()){
				cl->childs.insert(ptr);
			}
			else{
				Closure::ClosureIterator it = cl->childs.find(ptr);
				if (it != cl->childs.end())
					cl->childs.erase(it);
			}
		}

		if (operand) sp -= 2; //table init
		else sp -= 3;

		break;
	}
	case Opcode::EXIT:
		ip = code.size();
		break;
	case Opcode::NEG:{
		Value v = stack[sp - 1];
		if (v.IsTable()){
			overrideOp(v, "_neg", 0, "-");
		}
		else{
			stack[sp - 1] = -v;
		}

		break;
	}
	case Opcode::ADD:{
		Value v = stack[sp - 1];
		if (v.IsTable()){
			overrideOp(v, "_add", 1, "+");
		}
		else{
			stack[sp - 2] = v + stack[sp - 2];
			sp--;
		}

		break;
	}
	case Opcode::SUB:{
		Value v = stack[sp - 1];
		if (v.IsTable()){
			overrideOp(v, "_sub", 1, "-");
		}
		else{
			stack[sp - 2] = v - stack[sp - 2];
			sp--;
		}

		break;
	}
	case Opcode::MUL:{
		Value v = stack[sp - 1];
		if (v.IsTable()){
			overrideOp(v, "_mul", 1, "*");
		}
		else{
			stack[sp - 2] = v * stack[sp - 2];
			sp--;
		}

		break;
	}
	case Opcode::DIV:{
		Value v = stack[sp - 1];
		if (v.IsTable()){
			overrideOp(v, "_div", 1, "/");
		}
		else{
			stack[sp - 2] = v / stack[sp - 2];
			sp--;
		}

		break;
	}
	case Opcode::MOD:{
		Value v = stack[sp - 1];
		if (v.IsTable()){
			overrideOp(v, "_mod", 1, "%");
		}
		else{
			stack[sp - 2] = v % stack[sp - 2];
			sp--;
		}

		break;
	}
	case Opcode::PLUSEQ:{
		Value v = stack[sp - 1];
		if (v.IsTable()){
			overrideOp(v, "_pluseq", 1, "+=");
		}
		else{
			v += stack[sp - 2]; 
			stack[sp - 2] = v;
			sp--;
			move(operand, ins.operands);
		}

		break;
	}
	case Opcode::MIMUSEQ:{
		Value v = stack[sp - 1];
		if (v.IsTable()){
			overrideOp(v, "_minuseq", 1, "-=");
		}
		else{
			v -= stack[sp - 2];
			stack[sp - 2] = v;
			sp--;
			move(operand, ins.operands);
		}

		break;
	}
	case Opcode::MULEQ:{
		Value v = stack[sp - 1];
		if (v.IsTable()){
			overrideOp(v, "_muleq", 1, "*=");
		}
		else{
			v *= stack[sp - 2];
			stack[sp - 2] = v;
			sp--;
			move(operand, ins.operands);
		}

		break;
	}
	case Opcode::DIVEQ:{
		Value v = stack[sp - 1];
		if (v.IsTable()){
			overrideOp(v, "_diveq", 1, "/=");
		}
		else{
			v /= stack[sp - 2];
			stack[sp - 2] = v;
			sp--;
			move(operand, ins.operands);
		}

		break;
	}
	case Opcode::MODEQ:{
		Value v = stack[sp - 1];
		if (v.IsTable()){
			overrideOp(v, "_modeq", 1, "%=");
		}
		else{
			v %= stack[sp - 2];
			stack[sp - 2] = v;
			sp--;
			move(operand, ins.operands);
		}

		break;
	}
	case Opcode::GT:{
		Value v = stack[sp - 1];
		if (v.IsTable()){
			overrideOp(v, "_gt", 1, ">");
		}
		else{
			stack[sp - 2] = v > stack[sp - 2];
			sp--;
		}

		break;
	}
	case Opcode::LT:{
		Value v = stack[sp - 1];
		if (v.IsTable()){
			overrideOp(v, "_lt", 1, "<");
		}
		else{
			stack[sp - 2] = v < stack[sp - 2];
			sp--;
		}

		break;
	}
	case Opcode::GE:{
		Value v = stack[sp - 1];
		if (v.IsTable()){
			overrideOp(v, "_ge", 1, ">=");
		}
		else{
			stack[sp - 2] = v >= stack[sp - 2];
			sp--;
		}

		break;
	}
	case Opcode::LE:{
		Value v = stack[sp - 1];
		if (v.IsTable()){
			overrideOp(v, "_le", 1, "<=");
		}
		else{
			stack[sp - 2] = v <= stack[sp - 2];
			sp--;
		}

		break;
	}
	case Opcode::EQ:{
		Value v = stack[sp - 1];
		if (v.IsTable()){
			overrideOp(v, "_equal", 1, "==");
		}
		else{
			stack[sp - 2] = v == stack[sp - 2];
			sp--;
		}

		break;
	}
	case Opcode::NE:{
		Value v = stack[sp - 1];
		if (v.IsTable()){
			overrideOp(v, "_nequal", 1, "!=");
		}
		else{
			stack[sp - 2] = v != stack[sp - 2];
			sp--;
		}

		break;
	}
	case Opcode::OR:{
		Value v = stack[sp - 1];
		if (v.IsTable()){
			overrideOp(v, "_or", 1, "||");
		}
		else{
			stack[sp - 2] = v || stack[sp - 2];
			sp--;
		}

		break;
	}
	case Opcode::AND:{
		Value v = stack[sp - 1];
		if (v.IsTable()){
			overrideOp(v, "_and", 1, "&&");
		}
		else{
			stack[sp - 2] = v && stack[sp - 2];
			sp--;
		}

		break;
	}
	case Opcode::NOP: //do nothing
		break;
	}

	ip++;
}

void SVM::constructTDot(Tptr t, int fp, int ap){
	t->AddInt("num", ap - fp + 1);
	for (int i = fp - 1; i < ap; ++i){
		Value p = stack[cp + i];
		t->AddValue(to_string(i - fp + 1), p);
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
			Value vof = stack[tcp + tap + 3];
			Value vap = stack[tcp + tap + 4];
			Value vfp = stack[tcp + tap + 5];
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
			Value vof = stack[tcp + tap + 3];
			Value vap = stack[tcp + tap + 4];
			Value vfp = stack[tcp + tap + 5];
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

Clptr SVM::createClosure(Clptr o){
	Clptr f = shared_ptr<Closure>(new Closure());
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

	return f;
}

void SVM::overrideOp(Value t, string opname, int np, string op){
	if (t.GetTable()->HasValue(opname)){
		Value func = t.GetTable()->GetValue(opname);
		sp--;
		
		PushStack(func);
		CallScript(np);
	}
	else{
		if (t.GetTable()->HasValue("_default")){
			Value func = t.GetTable()->GetValue("_default");
			sp--;

			PushString(op);
			PushStack(func);
			CallScript(1);
		}
		else{
			Error::GetInstance()->ProcessError("没有对应的操作符[%s]", op.c_str());
		}
	}
}

void SVM::move(int operand, string operands){
	bool closure = operand & 0x40000000;
	int idx = operand & 0x3fffffff;
	if (closure){
		if (cl->ocvs.find(operands) != cl->ocvs.end()){
			cl->ocvs[operands] = stack[sp - 1];
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
		"PLUSEQ",
		"MINUSEQ",
		"MULEQ",
		"DIVEQ",
		"MODEQ",
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
		"PUSHN",
		"GETLEN",
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