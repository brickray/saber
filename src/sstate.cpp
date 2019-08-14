#include "sstate.h"

void SState::Init(){
	lexer = shared_ptr<Lexer>(new Lexer());
	parse = shared_ptr<SyntaxParse>(new SyntaxParse());
	svm = shared_ptr<SVM>(new SVM());
	env = shared_ptr<Environment>(new Environment());
	
	NativeFunc::Register(env);
}

void SState::Run(string code){
	lexer->Parse(code);
	parse->Parse(*lexer);
	parse->Compile(env, svm);
	svm->Run();
}

void SState::ShowCode() const{
	printf("%s\n", svm->ShowCode().c_str());
}

void SState::Register(RegisterFunction func[]){
	for (int i = 0;; ++i){
		string name = func[i].name;
		if (name == "") break;

		SValue sv;
		sv.sfunc = func[i].f;
		Value print(EValueType::ENATIVEFUNC, sv);
		SymbolInfo si = { print, int(sv.sfunc) };
		env->SetSymbol(name, si);
	}
}