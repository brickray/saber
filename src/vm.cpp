#include "vm.h"
#include "opcode.h"

VM::VM(){
	code.reserve(64);
	registers.reserve(NUM_REGISTER);
	pc = 0;
}

int VM::AddCode(int c){
	code.push_back(c);

	return code.size() - 1;
}

void VM::SetCode(int idx, int c){
	code[idx] = c;
}

int VM::AddConstant(Value v){
	constant.push_back(v);

	return encodeConstantIndex(constant.size());
}

int VM::AddStack(Value v){
	stack.push_back(v);

	return stack.size() - 1;
}

int VM::AddRegister(Value v){
	registers.push_back(v);

	return encodeRegisterIndex(registers.size());
}

void VM::Run(){
	int numCode = code.size();
	while (pc < numCode){
		int op = code[pc];
		switch (op){
		case Opcode::LOAD:
			break;
		case Opcode::MOVE:
			move();
			break;
		case Opcode::JZ:
			jz();
			break;
		case Opcode::JUMP:
			jump();
			break;
		case Opcode::CALL:
			call();
			break;
		case Opcode::CALLN:
			calln();
			break;
		case Opcode::ADD:
			add();
			break;
		case Opcode::SUB:
			sub();
			break;
		case Opcode::MUL:
			mul();
			break;
		case Opcode::DIV:
			div();
			break;
		case Opcode::MOD:
			mod();
			break;
		case Opcode::GT:
			gt();
			break;
		case Opcode::GE:
			gte();
			break;
		case Opcode::EQ:
			equal();
			break;
		case Opcode::NE:
			nequal();
			break;
		case Opcode::NOP: //只是将代码计数器指向下一条代码
			pc++;
			break;
		}
	}
}

void VM::move(){
	int destIdx = code[pc + 1];
	int srcIdx = code[pc + 2];
	Value* dest;
	if (isStack(destIdx)){
		dest = &stack[destIdx];
	}
	else{
		dest = &registers[decodeRegisterIndex(destIdx)];
	}

	if (isStack(srcIdx)){
		*dest = stack[srcIdx];
	}
	else if (isRegister(srcIdx)){
		*dest = registers[decodeRegisterIndex(srcIdx)];
	}
	else{
		*dest = constant[decodeConstantIndex(srcIdx)];
	}

	pc += 3;
}

void VM::jz(){
	int condIdx = code[pc + 1];
	int jump = code[pc + 2];
	Value* cond;
	if (isStack(condIdx)){
		cond = &stack[condIdx];
	}
	else if (isRegister(condIdx)){
		cond = &registers[decodeRegisterIndex(condIdx)];
	}
	else{
		cond = &registers[decodeRegisterIndex(condIdx)];
	}

	bool b = false;
	if (cond->IsBoolean()) b = cond->GetBoolean();
	else if (cond->IsInteger()) b = cond->GetInteger() != 0;
	else if (cond->IsFloat()) b = cond->GetFloat() != 0;

	if (b) pc += 3;
	else pc = jump;
}

void VM::jump(){
	int j = code[pc + 1];
	pc = j;
}

void VM::call(){

}

void VM::calln(){
	SFunc func = SFunc(code[pc + 1]);
	int numParams = code[pc + 2];
	for (int i = 0; i < numParams; ++i){
		
	}

	func(this);
	pc += 3 + numParams;
}

void VM::add(){
	int destIdx = code[pc + 1];
	int srcIdx = code[pc + 2];
	Value* dest;
	if (isStack(destIdx)){
		dest = &stack[destIdx];
	}
	else{
		dest = &registers[decodeRegisterIndex(destIdx)];
	}
	if (isStack(srcIdx)){
		*dest += stack[srcIdx];
	}
	else if (isRegister(srcIdx)){
		*dest += registers[decodeRegisterIndex(srcIdx)];
	}
	else{
		*dest += constant[decodeConstantIndex(srcIdx)];
	}

	pc += 3;
}

void VM::sub(){
	int destIdx = code[pc + 1];
	int srcIdx = code[pc + 2];
	Value* dest;
	if (isStack(destIdx)){
		dest = &stack[destIdx];
	}
	else{
		dest = &registers[decodeRegisterIndex(destIdx)];
	}
	if (isStack(srcIdx)){
		*dest -= stack[srcIdx];
	}
	else if (isRegister(srcIdx)){
		*dest -= registers[decodeRegisterIndex(srcIdx)];
	}
	else{
		*dest -= constant[decodeConstantIndex(srcIdx)];
	}

	pc += 3;
}

void VM::mul(){
	int destIdx = code[pc + 1];
	int srcIdx = code[pc + 2];
	Value* dest;
	if (isStack(destIdx)){
		dest = &stack[destIdx];
	}
	else{
		dest = &registers[decodeRegisterIndex(destIdx)];
	}
	if (isStack(srcIdx)){
		*dest *= stack[srcIdx];
	}
	else if (isRegister(srcIdx)){
		*dest *= registers[decodeRegisterIndex(srcIdx)];
	}
	else{
		*dest *= constant[decodeConstantIndex(srcIdx)];
	}

	pc += 3;
}

void VM::div(){
	int destIdx = code[pc + 1];
	int srcIdx = code[pc + 2];
	Value* dest;
	if (isStack(destIdx)){
		dest = &stack[destIdx];
	}
	else{
		dest = &registers[decodeRegisterIndex(destIdx)];
	}
	if (isStack(srcIdx)){
		*dest /= stack[srcIdx];
	}
	else if (isRegister(srcIdx)){
		*dest /= registers[decodeRegisterIndex(srcIdx)];
	}
	else{
		*dest /= constant[decodeConstantIndex(srcIdx)];
	}

	pc += 3;
}

void VM::mod(){
	int destIdx = code[pc + 1];
	int srcIdx = code[pc + 2];
	Value* dest;
	if (isStack(destIdx)){
		dest = &stack[destIdx];
	}
	else{
		dest = &registers[decodeRegisterIndex(destIdx)];
	}
	if (isStack(srcIdx)){
		*dest %= stack[srcIdx];
	}
	else if (isRegister(srcIdx)){
		*dest %= registers[decodeRegisterIndex(srcIdx)];
	}
	else{
		*dest %= constant[decodeConstantIndex(srcIdx)];
	}

	pc += 3;
}

void VM::gt(){
	int regIdx = code[pc + 1];
	int destIdx = code[pc + 2];
	int srcIdx = code[pc + 3];
	Value* dest, *src;
	if (isStack(destIdx)){
		dest = &stack[destIdx];
	}
	else if (isRegister(destIdx)){
		dest = &registers[decodeRegisterIndex(destIdx)];
	}
	else{
		dest = &registers[decodeRegisterIndex(destIdx)];
	}
	if (isStack(srcIdx)){
		src = &stack[srcIdx];
	}
	else if (isRegister(srcIdx)){
		src = &registers[decodeRegisterIndex(srcIdx)];
	}
	else{
		src = &constant[decodeConstantIndex(srcIdx)];
	}
	
	registers[decodeRegisterIndex(regIdx)] = *dest > *src;

	pc += 4;
}

void VM::gte(){
	int regIdx = code[pc + 1];
	int destIdx = code[pc + 2];
	int srcIdx = code[pc + 3];
	Value* dest, *src;
	if (isStack(destIdx)){
		dest = &stack[destIdx];
	}
	else if (isRegister(destIdx)){
		dest = &registers[decodeRegisterIndex(destIdx)];
	}
	else{
		dest = &registers[decodeRegisterIndex(destIdx)];
	}
	if (isStack(srcIdx)){
		src = &stack[srcIdx];
	}
	else if (isRegister(srcIdx)){
		src = &registers[decodeRegisterIndex(srcIdx)];
	}
	else{
		src = &constant[decodeConstantIndex(srcIdx)];
	}

	registers[decodeRegisterIndex(regIdx)] = *dest >= *src;

	pc += 4;
}

void VM::equal(){
	int regIdx = code[pc + 1];
	int destIdx = code[pc + 2];
	int srcIdx = code[pc + 3];
	Value* dest, *src;
	if (isStack(destIdx)){
		dest = &stack[destIdx];
	}
	else if (isRegister(destIdx)){
		dest = &registers[decodeRegisterIndex(destIdx)];
	}
	else{
		dest = &registers[decodeRegisterIndex(destIdx)];
	}
	if (isStack(srcIdx)){
		src = &stack[srcIdx];
	}
	else if (isRegister(srcIdx)){
		src = &registers[decodeRegisterIndex(srcIdx)];
	}
	else{
		src = &constant[decodeConstantIndex(srcIdx)];
	}

	registers[decodeRegisterIndex(regIdx)] = *dest == *src;

	pc += 4;
}

void VM::nequal(){
	int regIdx = code[pc + 1];
	int destIdx = code[pc + 2];
	int srcIdx = code[pc + 3];
	Value* dest, *src;
	if (isStack(destIdx)){
		dest = &stack[destIdx];
	}
	else if (isRegister(destIdx)){
		dest = &registers[decodeRegisterIndex(destIdx)];
	}
	else{
		dest = &registers[decodeRegisterIndex(destIdx)];
	}
	if (isStack(srcIdx)){
		src = &stack[srcIdx];
	}
	else if (isRegister(srcIdx)){
		src = &registers[decodeRegisterIndex(srcIdx)];
	}
	else{
		src = &constant[decodeConstantIndex(srcIdx)];
	}

	registers[decodeRegisterIndex(regIdx)] = *dest != *src;

	pc += 4;
}

bool VM::isStack(int idx){
	return idx >= 0;
}

bool VM::isRegister(int idx){
	return idx < 0 && idx > -NUM_REGISTER;
}

int VM::encodeRegisterIndex(int idx){
	return -idx;
}

int VM::decodeRegisterIndex(int idx){
	return -idx - 1;
}

int VM::encodeConstantIndex(int idx){
	return -idx - NUM_REGISTER;
}

int VM::decodeConstantIndex(int idx){
	return -(idx + NUM_REGISTER + 1);
}

string VM::ShowCode(){
	static string codeString[Opcode::ECNUM] = {
		"LOAD",
		"MOVE",
		"JZ",
		"JUMP",
		"CALL",
		"CALLN",
		"RET",
		"SAVE",
		"RESTORE",
		"NEG",
		"ADD",
		"SUB",
		"MUL",
		"DIV",
		"MOD",
		"GT",
		"GE",
		"EQ",
		"NE",
		"NOP",
	};

	string ret;
	for (int i = 0; i < code.size(); ++i){
		string c = codeString[code[i]];
		ret += c;
		if (c == "JUMP"){
			ret += "  ";
			ret += to_string(code[++i]);
		}
		else if (c == "MOVE" || c == "ADD" || c == "SUB" ||
			c == "MUL" || c == "DIV" || c == "MOD" ||
			c == "JZ"){
			ret += "  ";
			ret += to_string(code[++i]);
			ret += "  ";
			ret += to_string(code[++i]);
		}
		else if (c == "GT" || c == "LT" || c == "GTE" ||
			c == "LTE" || c == "EQUAL" || c == "NEQUAL"){
			ret += "  ";
			ret += to_string(code[++i]);
			ret += "  ";
			ret += to_string(code[++i]);
			ret += "  ";
			ret += to_string(code[++i]);
		}
		else if (c == "CALLN"){
			ret += "  ";
			ret += to_string(code[++i]);
			ret += "  ";
			int num = code[++i];
			ret += to_string(num);
			ret += "  ";
			for (int j = 0; j < num; ++j){
				ret += to_string(code[++i]);
				ret += "  ";
			}
		}
		
		ret += "\n";
	}

	return ret;
}