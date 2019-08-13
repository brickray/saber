#include "nativeFunc.h"
#include "astree.h"

static Value print(VM* vm){
	/*int numParams = astree->GetNumChildren() - 1;
	string ret;
	for (int i = 0; i < numParams; ++i){
		int v = astree->GetChild(i + 1)->Eval(e, vm);
		ret += v.ToString();
		ret += " ";
	}

	printf("%s\n", ret.c_str());*/
	printf("asdf\n");
	return Value();
}

static Value sin(VM* vm){
	return Value();

	/*Value v = astree->GetChild(1)->Eval(e, vm);
	float s = sin(v.GetInteger() * 3.1415926 / 180);
	SValue sv;
	sv.fValue = s;
	Value ret(EValueType::EFLOAT, sv);
	return ret;*/
}

static Value cos(VM* vm){
	return Value();
}

static Value tan(VM* vm){
	return Value();
}

static RegisterFunction native[] = {
	{ "print", print },
	{ "sin", sin },
	{ "cos", cos },
	{ "tan", tan },
	{ "", nullptr },
};

void NativeFunc::Register(shared_ptr<Environment>& e){
	for (int i = 0;; ++i){
		string name = native[i].name;
		if (name == "") break;

		SValue sv;
		sv.sfunc = native[i].f;
		Value print(EValueType::ENATIVEFUNC, sv);
		SymbolInfo si = { print, int(sv.sfunc) };
		e->SetSymbol(name, si);
	}
}