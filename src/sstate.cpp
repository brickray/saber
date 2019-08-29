#include "sstate.h"
#include "error.h"

SABER_NAMESPACE_BEGIN

void SState::Init(){
	preprocessor = shared_ptr<Preprocessor>(new Preprocessor());
	lexer = shared_ptr<Lexer>(new Lexer());
	parse = shared_ptr<SyntaxParse>(new SyntaxParse());
	svm = shared_ptr<SVM>(new SVM(this));
	env = shared_ptr<Environment>(new Environment());
	
	NativeFunc::Register(env, svm);
}

void SState::Compile(string code){
	string afterProcess = preprocessor->Process(code);
	lexer->Parse(afterProcess);
	parse->Parse(*lexer);
	parse->Compile(env, svm);

	if (sc) ShowCode();
}

void SState::Run(){
	svm->Run();
}

void SState::ShowCode(bool t){
	sc = t;
}

void SState::ShowCode() const{
	printf("%s\n", svm->ShowCode().c_str());
}

void SState::PushBool(bool b){
	svm->PushBool(b);
}

void SState::PushInt(int i){
	svm->PushInt(i);
}

void SState::PushFloat(float f){
	svm->PushFloat(f);
}

void SState::PushString(string s){
	svm->PushString(s);
}

void SState::PushFunction(string str){
	if (env->HasSymbol(str)){
		SymbolInfo si = env->GetSymbol(str);
		if (si.value.IsFunction()){
			svm->PushFunc(si.value.GetFunction());
		}
	}
}

void SState::CallScript(int nps){
	svm->CallScript(nps);
}

Value SState::PopStack(){
	return svm->PopStack();
}

void SState::Register(RegisterFunction func[]){
	for (int i = 0;; ++i){
		string name = func[i].name;
		if (name == "") break;

		Value function;
		function.SetNativeFunction(func[i].f);
		int idx = svm->AddGlobal(function);
		SymbolInfo si = { function, idx };
		env->SetSymbol(name, si);
	}
}

SABER_NAMESPACE_END