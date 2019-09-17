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
		int p = curCl->entry;
		int nfp = curCl->fp;
		bool vararg = curCl->vararg;
		int nap = numParams;
		int ncp = sp - nap;
		int nextip = ip + 1;
		if ((!vararg) && (nfp != nap)){
			Error::GetInstance()->ProcessError("形参和实参数量不匹配");
		}

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

		curCl->cp = cp;
		curCl->of = offset;
		curCl->ap = ap;
		if (vararg){
			Tptr t = Tptr(new Table());
			stack[sp - 1].SetTable(t);
			//construct ...
			constructTDot(t, fp, ap);
		}

		cl = curCl;
		ip = p;

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
		move(ins);

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
	case Opcode::JUMP:{
		ip = operand;

		return;
	}
	case Opcode::CALL:{
		Value func = stack[sp - 1];
		sp--; //pop
		if (func.IsNativeFunction()){
			func.GetNativeFunction()(this, operand);
			break;
		}
		else if (func.IsFunction()){
			Clptr curCl = func.GetFunction();
			int p = curCl->entry;
			int nfp = curCl->fp;
			bool vararg = curCl->vararg;
			int nap = operand;
			int ncp = sp - nap;
			if ((!vararg) && (nfp != nap)){
				Error::GetInstance()->ProcessError("形参和实参数量不匹配");
			}

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

			curCl->cp = cp;
			curCl->of = offset;
			curCl->ap = ap;
			if (vararg){
				Tptr t = Tptr(new Table());
				stack[sp - 1].SetTable(t);
				//construct ...
				constructTDot(t, fp, ap);
			}

			cl = curCl;
			ip = p;
			return;
		}
		else{
			Error::GetInstance()->ProcessError("尝试对[%s]值进行函数调用\n", func.GetTypeString().c_str());
			break;
		}
	}
	case Opcode::RET:{
		Value ret = stack[sp - 1];
		int numRetVariable = operand;
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

		//set closure value
		//只有返回值为table类型时,函数内定义的函数才创建闭包，因为只有这个时候闭包才有意义
		if (numRetVariable && ret.IsTable()){
			Tptr table = ret.GetTable();
			TableIteration it = table->Begin();
			for (; it != table->End(); ++it){
				if (it->second.IsFunction()){
					Clptr r = it->second.GetFunction();
					if (r->hascv && cl->cls.find(r) != cl->cls.end())
						it->second.SetFunction(createClosure(r));
				}
			}
		}

		//返回值为function类型时，进行闭包设置
		if (numRetVariable && ret.IsFunction() ){
			Clptr cur = ret.GetFunction();
			if (cur->hascv && cur != cl){
				Clptr r = createClosure(ret.GetFunction());
				ret.SetFunction(r);
				for (auto c : r->cls){
					c->parent = r;
				}
			}
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
			src = *getAddress(ins);
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
			Error::GetInstance()->ProcessError("尝试对[%s]使用[.], key:[%s]", table.GetTypeString().c_str(), key.GetString().c_str());
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
		Tptr t = Tptr(new Table());
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
			Error::GetInstance()->ProcessError("尝试对[%s]使用[.], key:[%s]", table.GetTypeString().c_str(), key.GetString().c_str());
		}
		if (!key.IsString() && !key.IsInteger()){
			Error::GetInstance()->ProcessError("key必须为integer或string");
		}
		Tptr t = table.GetTable();
		string s = key.GetString();
		if (key.IsInteger()) s = to_string(key.GetInteger());
		t->AddValue(s, value);

		if (operand) sp -= 2; //table init
		else sp -= 3;

		break;
	}
	case Opcode::EXIT:{
		ip = code.size();
		break;
	}
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
			move(ins);
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
			move(ins);
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
			move(ins);
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
			move(ins);
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
			move(ins);
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
		if (v.IsTable() && (v.GetTable()->HasValue("_equal") || v.GetTable()->HasValue("_default"))){
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
		if (v.IsTable() && (v.GetTable()->HasValue("_equal") || v.GetTable()->HasValue("_default"))){
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
	case Opcode::NOP:{//do nothing
		break;
	}
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

int SVM::getAbsoluteAddress(int op){
	int level = (op & 0xff000000) >> 24;
	int idx = op & 0x00ffffff;
	if (isStack(idx)){
		int tcp = cp;
		int tap = ap;
		int tfp = fp;
		int tof = offset;
		for (int i = 0; i < level - 1; ++i){
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
		return o;
	}
	else{
		return idx;
	}
}

Clptr SVM::createClosure(Clptr o){
	Clptr f = Clptr(new Closure(*o));
	f->parent = cl->parent;
	for (auto it : cl->cvs){
		f->cvs[it.first] = it.second;
	}
	for (auto it : cl->variables){
		Value v;
		int idx = it.second;
		if (isStack(idx)){
			int o = cp + idx + ((idx >= fp + 3) ? offset : 0);
			v = stack[o];
		}
		else{
			v = global[decodeGlobalIndex(idx)];
		}

		f->cvs[it.first] = v;
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

Value* SVM::getAddress(SVM::Instruction ins){
	Value* ret;
	int operand = ins.operand;
	string operands = ins.operands;
	bool closure = operand & 0x40000000;
	int idx = operand & 0x3fffffff;
	if (closure){
		Clptr p = cl;
		while (true){
			if (p){
				if (p->cvs.find(operands) != p->cvs.end()){
					ret = &p->cvs[operands];
					break;
				}
				else if (p->variables.find(operands) != p->variables.end()){
					idx = p->variables[operands];
					if (isStack(idx)){
						int o = p->cp + idx + ((idx >= p->fp + 3) ? p->of : 0);
						ret = &stack[o];
					}
					else{
						ret = &global[decodeGlobalIndex(idx)];
					}

					break;
				}
				else{
					p = p->parent;
				}
			}
		}
	}
	else{
		if (isStack(idx)){
			int o = cp + idx + ((idx >= fp + 3) ? offset : 0);
			ret = &stack[o];
		}
		else{
			ret = &global[decodeGlobalIndex(idx)];
		}
	}

	return ret;
}

void SVM::move(SVM::Instruction ins){
	*getAddress(ins) = stack[sp - 1];

	sp--;
}

void SVM::dumpStack(){
	string str = "";
	for (int i = 0; i < sp; i++){
		Value v = stack[i];
		str += to_string(i) + ". " + v.GetTypeString();
		str += ":";
		str += v.ToString();
		str += '\n';
	}

	printf("%s\nip[%d], sp[%d], cp[%d]\n", str.c_str(), ip, sp, cp);
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