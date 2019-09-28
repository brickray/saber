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

void SVM::PushNull(){
	Value v;
	PushStack(v);
}

void SVM::PushBool(bool b){
	Value v;
	v.SetBool(b);
	PushStack(v);
}

void SVM::PushInt(Integer i){
	Value v;
	v.SetInt(i);
	PushStack(v);
}

void SVM::PushFloat(Float f){
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

void SVM::PushLightUData(void* p){
	Value v;
	v.SetLightUData(p);
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
	//程序末尾加入结束命令
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
			if (nfp > nap){
				//如果形参大于实参，则用null补齐
				int delta = nfp - nap;
				for (int i = 0; i < delta; ++i){
					stack[sp++].SetNull();
				}
			}
			else{
				//如果形参小于实参，则舍弃多余参数
				int delta = nap - nfp;
				for (int i = 0; i < delta; ++i){
					--sp;
				}
			}

			nap = nfp;
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
		Error::GetInstance()->ProcessError("尝试对[%s]值进行函数调用\n", func.GetTypeString().c_str());
	}
}

void SVM::execute(){
	//使用引用避免拷贝构造，可大幅度提升速度
	Instruction& ins = code[ip];
	switch (ins.opcode){
	case Opcode::MOVE:{
		moveto(ins);

		break;
	}
	case Opcode::JZ:{
		Value& v = stack[--sp];
		//栈顶值为false则跳转
		if (isfalse(v)){
			ip = ins.operand;
			return;
		}

		break;
	}
	case Opcode::JNZ:{
		Value& v = stack[--sp];
		//栈顶值为true则跳转
		if (!isfalse(v)){
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
				if (nfp > nap){
					//如果形参大于实参，则用null补齐
					int delta = nfp - nap;
					for (int i = 0; i < delta; ++i){
						stack[sp++].SetNull();
					}
				}
				else{
					//如果形参小于实参，则舍弃多余参数
					int delta = nap - nfp;
					for (int i = 0; i < delta; ++i){
						--sp;
					}
				}

				nap = nfp;
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
			Error::GetInstance()->ProcessError("尝试对[%s]值进行函数调用\n", func.GetTypeString().c_str());
			break;
		}
	}
	case Opcode::TAILCALL:{
		Value& func = stack[--sp];
		Clptr curCl = func.GetFunction();
		int p = curCl->entry;
		int ncp = sp - ap;
		sp = ncp;

		//额,开始以为通过memcpy可以避免拷贝构造进行加速,
		//后来仔细一想这造成了另外的问题,引用计数没法正常工作了
		//因为智能指针正是通过拷贝构造对指针的引用次数进行操作的
		//所以老老实实改回原版,额。。。 以下同理
		//memcpy(&stack[cp], &stack[ncp], sizeof(Value)*ap);
		for (int i = 0; i < ap; ++i){
			stack[cp + i] = stack[ncp + i];
		}
	
		ip = p;
		return;
	}
	case Opcode::RET:{
		Value& ret = stack[sp - 1];
		int numRetVariable = ins.operand;
		int base = cp + ap;
		Tptr t = stack[base + TB_ADDRESS].GetTable();
		Clptr ocl = stack[base + CL_ADDRESS].GetFunction();
		int ofp = stack[base + FP_ADDRESS].GetInteger();
		int oap = stack[base + AP_ADDRESS].GetInteger();
		int oof = stack[base + OF_ADDRESS].GetInteger();
		int ocp = stack[base + CP_ADDRESS].GetInteger();
		int esp = stack[base + SP_ADDRESS].GetInteger();
		int eip = stack[base + IP_ADDRESS].GetInteger();
		bool isCoroutine = eip & 0x80000000;
		eip = eip & 0x7fffffff;

		//set closure value
		//只有返回值为table类型时,函数内定义的函数才创建闭包，因为只有这个时候闭包才有意义
		if (numRetVariable && ret.IsTable()){
			Tptr table = ret.GetTable();
			createClosure(table);
		}

		//返回值为function类型时，进行闭包设置
		if (numRetVariable && ret.IsFunction()){
			Clptr cur = ret.GetFunction();
			if (cur->hascv && cur != cl){
				Clptr r = createClosure(ret.GetFunction());
				ret.SetFunction(r);
			}
		}

		cl = ocl;
		fp = ofp;
		ap = oap;
		of = oof;
		cp = ocp;
		sp = esp;
		ip = eip;
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
			Error::GetInstance()->ProcessError("尝试对类型%s使用#", v.GetTypeString().c_str());
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
			//这个位置必定是函数地址
			int operand = ins.operand;
			if (!operand) break;
			int o = sp - 1 - operand;
			Value v = stack[o];
			
			//memcpy(&stack[o], &stack[o + 1], sizeof(Value)*operand);
			for (int i = 0; i < operand; ++i){
				stack[o + i] = stack[o + 1 + i];
			}
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
		//预留局部变量
		sp += ins.operand;

		break;
	}
	case Opcode::GTFILED:{
		int operand = ins.operand;
		Value& key = stack[sp - 1];
		Value& table = stack[sp - 2];
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
		Value& r = stack[sp - 1];
		if (r.IsInteger()){
			r.SetInt(-r.GetInteger());
		}
		else if (r.IsFloat()){
			r.SetFloat(-r.GetFloat());
		}
		else{
			tryOverrideOp(r, 0, "_neg");
		}

		break;
	}
	case Opcode::ADD:{
		Value& l = stack[--sp];
		Value& r = stack[sp - 1];
		if (l.IsInteger() && r.IsInteger()){
			r.SetInt(l.GetInteger() + r.GetInteger());
		}
		else if (l.IsNumber() && r.IsNumber()){
			r.SetFloat(l.GetNumber() + r.GetNumber());
		}
		else if (l.IsString() && r.IsString()){
			r.SetString(l.GetString() + r.GetString());
		}
		else{
			tryOverrideOp(l, 1, "_add");
		}

		break;
	}
	case Opcode::SUB:{
		Value& l = stack[--sp];
		Value& r = stack[sp - 1];
		if (l.IsInteger() && r.IsInteger()){
			r.SetInt(l.GetInteger() - r.GetInteger());
		}
		else if (l.IsNumber() && r.IsNumber()){
			r.SetFloat(l.GetNumber() - r.GetNumber());
		}
		else{
			tryOverrideOp(l, 1, "_sub");
		}

		break;
	}
	case Opcode::MUL:{
		Value& l = stack[--sp];
		Value& r = stack[sp - 1];
		if (l.IsInteger() && r.IsInteger()){
			r.SetInt(l.GetInteger() * r.GetInteger());
		}
		else if (l.IsNumber() && r.IsNumber()){
			r.SetFloat(l.GetNumber() * r.GetNumber());
		}
		else{
			tryOverrideOp(l, 1, "_mul");
		}

		break;
	}
	case Opcode::DIV:{
		Value& l = stack[--sp];
		Value& r = stack[sp - 1];
		if (l.IsInteger() && r.IsInteger()){
			r.SetInt(l.GetInteger() / r.GetInteger());
		}
		else if (l.IsNumber() && r.IsNumber()){
			r.SetFloat(l.GetNumber() / r.GetNumber());
		}
		else{
			tryOverrideOp(l, 1, "_div");
		}

		break;
	}
	case Opcode::MOD:{
		Value& l = stack[--sp];
		Value& r = stack[sp - 1];
		if (l.IsInteger() && r.IsInteger()){
			r.SetInt(l.GetInteger() % r.GetInteger());
		}
		else{
			tryOverrideOp(l, 1, "_mod");
		}

		break;
	}
	case Opcode::PLUSEQ:{
		Value& l = stack[--sp];
		Value& r = stack[sp - 1];
		if (l.IsInteger() && r.IsInteger()){
			r.SetInt(l.GetInteger() + r.GetInteger());
			moveto(ins);
		}
		else if (l.IsNumber() && r.IsNumber()){
			r.SetFloat(l.GetNumber() + r.GetNumber());
			moveto(ins);
		}
		else if (l.IsString() && r.IsString()){
			r.SetString(l.GetString() + r.GetString());
			moveto(ins);
		}
		else{
			tryOverrideOp(l, 1, "_pluseq");
		}

		break;
	}
	case Opcode::MIMUSEQ:{
		Value& l = stack[--sp];
		Value& r = stack[sp - 1];
		if (l.IsInteger() && r.IsInteger()){
			r.SetInt(l.GetInteger() - r.GetInteger());
			moveto(ins);
		}
		else if (l.IsNumber() && r.IsNumber()){
			r.SetFloat(l.GetNumber() - r.GetNumber());
			moveto(ins);
		}
		else{
			tryOverrideOp(l, 1, "_minuseq");
		}

		break;
	}
	case Opcode::MULEQ:{
		Value& l = stack[--sp];
		Value& r = stack[sp - 1];
		if (l.IsInteger() && r.IsInteger()){
			r.SetInt(l.GetInteger() * r.GetInteger());
			moveto(ins);
		}
		else if (l.IsNumber() && r.IsNumber()){
			r.SetFloat(l.GetNumber() * r.GetNumber());
			moveto(ins);
		}
		else{
			tryOverrideOp(l, 1, "_muleq");
		}

		break;
	}
	case Opcode::DIVEQ:{
		Value& l = stack[--sp];
		Value& r = stack[sp - 1];
		if (l.IsInteger() && r.IsInteger()){
			r.SetInt(l.GetInteger() / r.GetInteger());
			moveto(ins);
		}
		else if (l.IsNumber() && r.IsNumber()){
			r.SetFloat(l.GetNumber() / r.GetNumber());
			moveto(ins);
		}
		else{
			tryOverrideOp(l, 1, "_diveq");
		}

		break;
	}
	case Opcode::MODEQ:{
		Value& l = stack[--sp];
		Value& r = stack[sp - 1];
		if (l.IsInteger() && r.IsInteger()){
			r.SetInt(l.GetInteger() % r.GetInteger());
			moveto(ins);
		}
		else{
			tryOverrideOp(l, 1, "_modeq");
		}

		break;
	}
	case Opcode::GT:{
		Value& l = stack[--sp];
		Value& r = stack[sp - 1];
		if (l.IsNumber() && r.IsNumber()){
			r.SetBool(l.GetNumber() > r.GetNumber());
		}
		else if (l.IsString() && r.IsString()){
			r.SetBool(l.GetString() > r.GetString());
		}
		else{
			tryOverrideOp(l, 1, "_gt");
		}

		break;
	}
	case Opcode::LT:{
		Value& l = stack[--sp];
		Value& r = stack[sp - 1];
		if (l.IsNumber() && r.IsNumber()){
			r.SetBool(l.GetNumber() < r.GetNumber());
		}
		else if (l.IsString() && r.IsString()){
			r.SetBool(l.GetString() < r.GetString());
		}
		else{
			tryOverrideOp(l, 1, "_lt");
		}

		break;
	}
	case Opcode::GE:{
		Value& l = stack[--sp];
		Value& r = stack[sp - 1];
		if (l.IsNumber() && r.IsNumber()){
			r.SetBool(l.GetNumber() >= r.GetNumber());
		}
		else if (l.IsString() && r.IsString()){
			r.SetBool(l.GetString() >= r.GetString());
		}
		else{
			tryOverrideOp(l, 1, "_ge");
		}
		

		break;
	}
	case Opcode::LE:{
		Value& l = stack[--sp];
		Value& r = stack[sp - 1];
		if (l.IsNumber() && r.IsNumber()){
			r.SetBool(l.GetNumber() <= r.GetNumber());
		}
		else if (l.IsString() && r.IsString()){
			r.SetBool(l.GetString() <= r.GetString());
		}
		else{
			tryOverrideOp(l, 1, "_le");
		}

		break;
	}
	case Opcode::EQ:{
		Value& l = stack[--sp];
		Value& r = stack[sp - 1];
		r.SetBool(isequal(l, r));

		break;
	}
	case Opcode::NE:{
		Value& l = stack[--sp];
		Value& r = stack[sp - 1];
		r.SetBool(!isequal(l, r));

		break;
	}
	case Opcode::OR:{
		Value& r = stack[--sp];
		//或运算符左边肯定为false，所以运算结果取决于右边
		r.SetBool(!isfalse(r));
		sp++;

		break;
	}
	case Opcode::AND:{
		Value& r = stack[--sp];
		//与运算符左边肯定为true， 所以运算结果取决于右边
		r.SetBool(!isfalse(r));
		sp++;

		break;
	}
	case Opcode::NOT:{
		Value& r = stack[sp - 1];
		r.SetBool(isfalse(r));

		break;
	}
	case Opcode::NOP:{//do nothing
		break;
	}
	}

	ip++;
}

void SVM::constructTDot(Tptr t, int fp, int ap){
	//构建可变参
	t->AddInt("num", ap - fp + 1);
	for (int i = fp - 1; i < ap; ++i){
		Value& p = stack[cp + i];
		t->AddValue(to_string(i - fp + 1), p);
	}
}

void SVM::createClosure(Tptr table){
	TableIteration it = table->Begin();
	for (; it != table->End(); ++it){
		if (it->second.IsFunction()){
			Clptr r = it->second.GetFunction();
			if (r->hascv && cl->cls.find(r) != cl->cls.end())
				it->second.SetFunction(createClosure(r));
		}
		else if (it->second.IsTable()){
			//table内的table也可能有非局部变量
			//所以这里递归处理
			createClosure(it->second.GetTable());
		}
	}
}

Clptr SVM::createClosure(Clptr o){
	Clptr f = Clptr(new Closure(*o));
	for (auto it : cl->cvs){
		//将当前函数内的非局部变量复制到闭包
		f->cvs[it.first] = it.second;
	}
	for (auto it : cl->variables){
		//将当前函数内的变量复制到闭包
		Value& v = f->cvs[it.first];
		int idx = it.second;
		v = *getAddress(Instruction(Opcode::NOP, idx));
	}

	return f;
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
					//首先寻找闭包变量
					ret = &p->cvs[operands];
					break;
				}
				else if (p->variables.find(operands) != p->variables.end()){
					//再寻找函数内的局部变量
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
					//不能使用parent这一变量来寻找上一级
					//因为当碰到递归函数时会无限循环
					//只能通过栈去寻找
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

void SVM::moveto(SVM::Instruction& ins){
	*getAddress(ins) = stack[--sp];
}

bool SVM::isfalse(Value& v){
	bool t;
	if (v.IsBoolean()) t = !v.GetBoolean();
	else if (v.IsNumber()) t = v.GetNumber() == 0;
	else if (v.IsNull()) t = true;
	else t = false;

	return t;
}

bool SVM::isequal(Value& l, Value& r){
	if (l.IsNumber() && r.IsNumber()){
		//int和float类型有可能相等
		return l.GetNumber() == r.GetNumber();
	}

	if (l.GetType() != r.GetType()){
		//除了以上2中类型外,类型不相同肯定不相等
		return false;
	}
	else{
		//根据类型分别判断
		switch (l.GetType()){
		case EValueType::ENULL: return true;
		case EValueType::EBOOLEAN: return l.GetBoolean() == r.GetBoolean();
		case EValueType::ESTRING: return l.GetString() == r.GetString();
		case EValueType::EFUNC: return l.GetFunction() == r.GetFunction();
		case EValueType::ENATIVEFUNC: return l.GetNativeFunction() == r.GetNativeFunction();
		case EValueType::ELIGHTUDATA: return l.GetLightUData() == r.GetLightUData();
		case EValueType::ETABLE: return l.GetTable() == r.GetTable();
		case EValueType::ECOROUTINE: return l.GetCoroutine() == r.GetCoroutine();
		}
	}
}

void SVM::tryOverrideOp(Value& t, int np, const char* op){
	static hash_map<string, string> opmap = {
		{ "_neg", "-" },
		{ "_add", "+" },
		{ "_sub", "-" },
		{ "_mul", "*" },
		{ "_div", "/" },
		{ "_mod", "%" },
		{ "_pluseq", "+=" },
		{ "minuseq", "-=" },
		{ "_muleq", "*=" },
		{ "_diveq", "/=" },
		{ "_modeq", "%=" },
		{ "_gt", ">" },
		{ "_lt", "<" },
		{ "_ge", ">=" },
		{ "_le", "<=" }
	};

	if (t.IsTable()){
		if (t.GetTable()->HasValue(op)){
			Value& func = t.GetTable()->GetValue(op);
			PushStack(func);
			CallScript(np);
		}
		else{
			Error::GetInstance()->ProcessError("table没有重载操作符[%s]", opmap[op].c_str());
		}
	}
	else{
		if (!np){
			Error::GetInstance()->ProcessError("尝试对类型%s进行%s操作", t.GetTypeString().c_str(), opmap[op].c_str());
		}
		else{
			Error::GetInstance()->ProcessError("尝试对类型%s和%s进行%s操作", t.GetTypeString().c_str(), stack[sp - 1].GetTypeString().c_str(), opmap[op].c_str());
		}
	}
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
		"NOT",
		"SETTABLE",
		"STFILED",
		"EXIT",
		"PUSHN",
		"GETLEN",
		"NOP",
		"GTFILED",
		"MOVE",
		"JZ",
		"JNZ",
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