#include "svm.h"
#include "opcode.h"
#include "error.h"
#include "nativeFunc.h"

SABER_NAMESPACE_BEGIN

SVM::SVM(SState* s){
	S = s;

	code.reserve(1024);
	stack.resize(STACK_SIZE);
	global.reserve(128);
	ip = 0;
	sp = 0;
	cp = 0;
	of = 0;
	ap = 0;
	fp = 0;
	cl = nullptr;
}

int SVM::AddCode(Instruction& c){
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

void SVM::SetCode(int idx, Instruction& c){
	code[idx] = c;
}

int SVM::AddGlobal(Value& v){
	global.push_back(v);
	
	return encodeGlobalIndex(global.size() - 1);
}

void SVM::SetStack(int i, Value& v){
	stack[i] = v;
}

void SVM::PushStack(Value& v){
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

void SVM::PushCoroutine(Coptr co){
	Value v;
	v.SetCoroutine(co);
	PushStack(v);
}

Value SVM::PopStack(){
	return stack[--sp];
}

void SVM::PushCo(Coptr c) {
	co.push_back(c);
}

Coptr SVM::PopCo() {
	if (co.size() == 0)
		return nullptr;
	Coptr c = co[co.size() - 1]; 
	co.pop_back();
	return c;
}

void SVM::Run(){
	//����ĩβ�����������
	SVM::Instruction exit(Opcode::EXIT);
	AddCode(exit);
	codeSize = code.size();

	while (ip < codeSize){
		execute();
	}
}

void SVM::CallScript(int numParams){
	Value& func = stack[--sp];
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
			Error::GetInstance()->ProcessError("�βκ�ʵ��������ƥ��");
		}

		stack[sp + IP_ADDRESS].SetInt(nextip);
		stack[sp + SP_ADDRESS].SetInt(ncp);
		stack[sp + CP_ADDRESS].SetInt(cp);
		stack[sp + OF_ADDRESS].SetInt(of);
		stack[sp + AP_ADDRESS].SetInt(ap);
		stack[sp + FP_ADDRESS].SetInt(fp);
		stack[sp + CL_ADDRESS].SetFunction(cl);
		stack[sp + TB_ADDRESS].SetNull(); //variable parameters table
		sp += NUM_ADDRESS;

		fp = nfp;
		ap = nap;
		of = ap - fp;
		cp = ncp;

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
			if (ip >= codeSize) break;
		}
	}
	else{
		Error::GetInstance()->ProcessError("���Զ�[%s]ֵ���к�������\n", func.GetTypeString().c_str());
	}
}

void SVM::execute(){
	//ʹ�����ñ��⿽�����죬�ɴ���������ٶ�
	Instruction& ins = code[ip];
	switch (ins.opcode){
	case Opcode::MOVE:{
		move(ins);

		break;
	}
	case Opcode::JZ:{
		Value& v = stack[--sp];
		bool t;
		if (v.IsBoolean()) t = v.GetBoolean();
		else if (v.IsFloat()) t = v.GetFloat() != 0;
		else if (v.IsInteger()) t = v.GetInteger() != 0;
		else if (v.IsNull()) t = false;
		else break;

		if (!t){
			ip = ins.operand;
			return;
		}

		break;
	}
	case Opcode::JUMP:{
		ip = ins.operand;

		return;
	}
	case Opcode::CALL:{
		Value& func = stack[--sp];
		if (func.IsNativeFunction()){
			func.GetNativeFunction()(this, ins.operand);
			break;
		}
		else if (func.IsFunction()){
			Clptr curCl = func.GetFunction();
			int p = curCl->entry;
			int nfp = curCl->fp;
			bool vararg = curCl->vararg;
			int nap = ins.operand;
			int ncp = sp - nap;
			if ((!vararg) && (nfp != nap)){
				Error::GetInstance()->ProcessError("�βκ�ʵ��������ƥ��");
			}

			stack[sp + IP_ADDRESS].SetInt(ip + 1);
			stack[sp + SP_ADDRESS].SetInt(ncp);
			stack[sp + CP_ADDRESS].SetInt(cp);
			stack[sp + OF_ADDRESS].SetInt(of);
			stack[sp + AP_ADDRESS].SetInt(ap);
			stack[sp + FP_ADDRESS].SetInt(fp);
			stack[sp + CL_ADDRESS].SetFunction(cl);
			stack[sp + TB_ADDRESS].SetNull(); //variable parameters table
			sp += NUM_ADDRESS;

			fp = nfp;
			ap = nap;
			of = ap - fp;
			cp = ncp;

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
			Error::GetInstance()->ProcessError("���Զ�[%s]ֵ���к�������\n", func.GetTypeString().c_str());
			break;
		}
	}
	case Opcode::TAILCALL:{
		Value& func = stack[--sp];
		Clptr curCl = func.GetFunction();
		int p = curCl->entry;
		int ncp = sp - ap;
		sp = ncp;

		memcpy(&stack[cp], &stack[ncp], sizeof(Value)*ap);

		ip = p;
		break;
	}
	case Opcode::RET:{
		Value& ret = stack[sp - 1];
		int numRetVariable = ins.operand;
		int base = cp + ap;
		Tptr t       = stack[base + TB_ADDRESS].GetTable();
		Clptr ocl    = stack[base + CL_ADDRESS].GetFunction();
		int ofp      = stack[base + FP_ADDRESS].GetInteger();
		int oap      = stack[base + AP_ADDRESS].GetInteger();
		int oof      = stack[base + OF_ADDRESS].GetInteger();
		int ocp      = stack[base + CP_ADDRESS].GetInteger();
		int esp      = stack[base + SP_ADDRESS].GetInteger();
		int eip      = stack[base + IP_ADDRESS].GetInteger();
		bool isCoroutine = eip & 0x80000000;
		eip = eip & 0x7fffffff;

		//set closure value
		//ֻ�з���ֵΪtable����ʱ,�����ڶ���ĺ����Ŵ����հ�����Ϊֻ�����ʱ��հ���������
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

		//����ֵΪfunction����ʱ�����бհ�����
		if (numRetVariable && ret.IsFunction() ){
			Clptr cur = ret.GetFunction();
			if (cur->hascv && cur != cl){
				Clptr r = createClosure(ret.GetFunction());
				ret.SetFunction(r);
			}
		}

		cl     = ocl;
		fp     = ofp;
		ap     = oap;
		of     = oof;
		cp     = ocp;
		sp     = esp;
		ip     = eip;
		if (numRetVariable) stack[sp++] = ret;
			
		if (isCoroutine) cocallback(this);

		return;
	}
	case Opcode::GETLEN:{
		Value& v = stack[sp - 1];
		if (v.IsTable()){
			v.SetInt(v.GetTable()->GetLength());
		}
		else if (v.IsString()){
			v.SetInt(v.GetString().size());
		}
		else{
			Error::GetInstance()->ProcessError("���Զ�����%sʹ��#", v.GetTypeString().c_str());
		}

		break;
	}
	case Opcode::PUSHN:{
		stack[sp++].SetNull();
		break;
	}
	case Opcode::PUSHB:{
		stack[sp++].SetBool(ins.operand);
		break;
	}
	case Opcode::PUSHI:{
		stack[sp++].SetInt(ins.operand);
		break;
	}
	case Opcode::PUSHF:{
		stack[sp++].SetFloat(ins.operandf);
		break;
	}
	case Opcode::PUSHS:{
		stack[sp++].SetString(ins.operands);
		break;
	}
	case Opcode::PUSH:{
		if (ins.relative){
			//���λ�ñض��Ǻ�����ַ
			int operand = ins.operand;
			if (!operand) break;
			int o = sp - 1 - operand;
			Value v = stack[o];
			//memcpy���Ա��⿽������
			memcpy(&stack[o], &stack[o + 1], sizeof(Value)*operand);
			stack[sp - 1] = v;
		}
		else{
			stack[sp++] = *getAddress(ins);
		}
		break;
	}
	case Opcode::POP:{
		sp--;
		break;
	}
	case Opcode::RESERVE:{
		sp += ins.operand;
		break;
	}
	case Opcode::GTFILED:{
		int operand = ins.operand;
		Value& key = stack[sp - 1];
		Value& table = stack[sp - 2];
		if (!table.IsTable()){
			Error::GetInstance()->ProcessError("���Զ�[%s]ʹ��[.], key:[%s]", table.GetTypeString().c_str(), key.GetString().c_str());
		}
		if (!key.IsString() && !key.IsInteger()){
			Error::GetInstance()->ProcessError("key����Ϊinteger��string");
		}

		Tptr t = table.GetTable();
		string s = key.GetString();
		if (key.IsInteger()) s = to_string(key.GetInteger());
		Value value;
		if (!t->HasValue(s)){
			if (!operand && t->HasValue("_default")){
				Value& func = t->GetValue("_default");

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
		int operand = ins.operand;
		Value& key = stack[sp - 1];
		Value table = stack[sp - 2];
		Value value = stack[sp - 3];
		if (operand){//table init
			value = stack[sp - 2];
			table = stack[sp - 3];
		}

		if (!table.IsTable()){
			Error::GetInstance()->ProcessError("���Զ�[%s]ʹ��[.], key:[%s]", table.GetTypeString().c_str(), key.GetString().c_str());
		}
		if (!key.IsString() && !key.IsInteger()){
			Error::GetInstance()->ProcessError("key����Ϊinteger��string");
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
		Value& v = stack[sp - 1];
		if (v.IsTable()){
			overrideOp(v, "_neg", 0, "-");
		}
		else{
			stack[sp - 1] = -v;
		}

		break;
	}
	case Opcode::ADD:{
		Value& v1 = stack[--sp];
		Value& v2 = stack[sp - 1];
		if (v1.IsTable()){
			overrideOp(v1, "_add", 1, "+");
		}
		else{
			v2 = v1 + v2;
		}

		break;
	}
	case Opcode::SUB:{
		Value& v1 = stack[--sp];
		Value& v2 = stack[sp - 1];
		if (v1.IsTable()){
			overrideOp(v1, "_sub", 1, "-");
		}
		else{
			v2 = v1 - v2;
		}

		break;
	}
	case Opcode::MUL:{
		Value& v1 = stack[--sp];
		Value& v2 = stack[sp - 1];
		if (v1.IsTable()){
			overrideOp(v1, "_mul", 1, "*");
		}
		else{
			v2 = v1 * v2;
		}

		break;
	}
	case Opcode::DIV:{
		Value& v1 = stack[--sp];
		Value& v2 = stack[sp - 1];
		if (v1.IsTable()){
			overrideOp(v1, "_div", 1, "/");
		}
		else{
			v2 = v1 / v2;
		}

		break;
	}
	case Opcode::MOD:{
		Value& v1 = stack[--sp];
		Value& v2 = stack[sp - 1];
		if (v1.IsTable()){
			overrideOp(v1, "_mod", 1, "%");
		}
		else{
			v2 = v1 % v2;
		}

		break;
	}
	case Opcode::PLUSEQ:{
		Value& v = stack[--sp];
		if (v.IsTable()){
			overrideOp(v, "_pluseq", 1, "+=");
		}
		else{
			//�����������Ϊ�ַ���������˳�����Ҫ
			Value& v2 = stack[sp - 1];
			v2 = v + v2;
			move(ins);
		}

		break;
	}
	case Opcode::MIMUSEQ:{
		Value& v = stack[--sp];
		if (v.IsTable()){
			overrideOp(v, "_minuseq", 1, "-=");
		}
		else{
			Value& v2 = stack[sp - 1];
			v2 = v - v2;
			move(ins);
		}

		break;
	}
	case Opcode::MULEQ:{
		Value& v = stack[--sp];
		if (v.IsTable()){
			overrideOp(v, "_muleq", 1, "*=");
		}
		else{
			Value& v2 = stack[sp - 1];
			v2 = v * v2;
			move(ins);
		}

		break;
	}
	case Opcode::DIVEQ:{
		Value& v = stack[--sp];
		if (v.IsTable()){
			overrideOp(v, "_diveq", 1, "/=");
		}
		else{
			Value& v2 = stack[sp - 1];
			v2 = v / v2;
			move(ins);
		}

		break;
	}
	case Opcode::MODEQ:{
		Value& v = stack[--sp];
		if (v.IsTable()){
			overrideOp(v, "_modeq", 1, "%=");
		}
		else{
			Value& v2 = stack[sp - 1];
			v2 = v / v2;
			move(ins);
		}

		break;
	}
	case Opcode::GT:{
		Value& v = stack[--sp];
		if (v.IsTable()){
			overrideOp(v, "_gt", 1, ">");
		}
		else{
			Value& v2 = stack[sp - 1];
			v2 = v > v2;
		}

		break;
	}
	case Opcode::LT:{
		Value& v = stack[--sp];
		if (v.IsTable()){
			overrideOp(v, "_lt", 1, "<");
		}
		else{
			Value& v2 = stack[sp - 1];
			v2 = v < v2;
		}

		break;
	}
	case Opcode::GE:{
		Value& v = stack[--sp];
		if (v.IsTable()){
			overrideOp(v, "_ge", 1, ">=");
		}
		else{
			Value& v2 = stack[sp - 1];
			v2 = v >= v2;
		}

		break;
	}
	case Opcode::LE:{
		Value& v = stack[--sp];
		if (v.IsTable()){
			overrideOp(v, "_le", 1, "<=");
		}
		else{
			Value& v2 = stack[sp - 1];
			v2 = v <= v2;
		}

		break;
	}
	case Opcode::EQ:{
		Value& v = stack[--sp];
		if (v.IsTable() && (v.GetTable()->HasValue("_equal") || v.GetTable()->HasValue("_default"))){
			overrideOp(v, "_equal", 1, "==");
		}
		else{
			Value& v2 = stack[sp - 1];
			v2 = v == v2;
		}

		break;
	}
	case Opcode::NE:{
		Value& v = stack[--sp];
		if (v.IsTable() && (v.GetTable()->HasValue("_equal") || v.GetTable()->HasValue("_default"))){
			overrideOp(v, "_nequal", 1, "!=");
		}
		else{
			Value& v2 = stack[sp - 1];
			v2 = v != v2;
		}

		break;
	}
	case Opcode::OR:{
		Value& v = stack[--sp];
		if (v.IsTable()){
			overrideOp(v, "_or", 1, "||");
		}
		else{
			Value& v2 = stack[sp - 1];
			v2 = v || v2;
		}

		break;
	}
	case Opcode::AND:{
		Value& v = stack[--sp];
		if (v.IsTable()){
			overrideOp(v, "_and", 1, "&&");
		}
		else{
			Value& v2 = stack[sp - 1];
			v2 = v && v2;
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
		Value& p = stack[cp + i];
		t->AddValue(to_string(i - fp + 1), p);
	}
}

Clptr SVM::createClosure(Clptr o){
	Clptr f = Clptr(new Closure(*o));
	for (auto it : cl->cvs){
		f->cvs[it.first] = it.second;
	}
	for (auto it : cl->variables){
		Value& v = f->cvs[it.first];
		int idx = it.second;
		v = *getAddress(Instruction(Opcode::NOP, idx));
	}

	return f;
}

void SVM::overrideOp(Value& t, const char* opname, int np, const char* op){
	if (t.GetTable()->HasValue(opname)){
		Value& func = t.GetTable()->GetValue(opname);
		
		PushStack(func);
		CallScript(np);
	}
	else{
		if (t.GetTable()->HasValue("_default")){
			Value& func = t.GetTable()->GetValue("_default");

			PushString(op);
			PushStack(func);
			CallScript(1);
		}
		else{
			Error::GetInstance()->ProcessError("û�ж�Ӧ�Ĳ�����[%s]", op);
		}
	}
}

Value* SVM::getAddress(SVM::Instruction& ins){
	Value* ret;
	int operand = ins.operand;
	bool closure = operand & 0x40000000;
	if (closure){
		Clptr p = cl;
		string& operands = ins.operands;
		int tcp = cp;
		int tap = ap;
		int tfp = fp;
		int tof = of;
		while (true){
			if (p){
				if (p->cvs.find(operands) != p->cvs.end()){
					ret = &p->cvs[operands];
					break;
				}
				else if (p->variables.find(operands) != p->variables.end()){
					int idx = p->variables[operands];
					if (isStack(idx)){
						int o = tcp + idx;
						if(tof) o += (idx >= tfp + 3) ? tof : 0;
						ret = &stack[o];
					}
					else{
						ret = &global[decodeGlobalIndex(idx)];
					}

					break;
				}
				else{
					//����ʹ��parent��һ������Ѱ����һ��
					//��Ϊ�������ݹ麯��ʱ������ѭ��
					//ֻ��ͨ��ջȥѰ��
					int base = tcp + tap;
					p   = stack[base + CL_ADDRESS].GetFunction();
					tof = stack[base + OF_ADDRESS].GetInteger();
					tfp = stack[base + FP_ADDRESS].GetInteger();
					tap = stack[base + AP_ADDRESS].GetInteger();
					tcp = stack[base + CP_ADDRESS].GetInteger();
				}
			}
		}
	}
	else{
		if (isStack(operand)){
			int o = cp + operand;
			if (of) o += ((operand >= fp + 3) ? of : 0);
			ret = &stack[o];
		}
		else{
			ret = &global[decodeGlobalIndex(operand)];
		}
	}

	return ret;
}

void SVM::move(SVM::Instruction& ins){
	*getAddress(ins) = stack[--sp];
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
		"TAILCALL",
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