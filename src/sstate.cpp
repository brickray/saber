#include "sstate.h"

SABER_NAMESPACE_BEGIN

void SState::Init(){
	lexer = shared_ptr<Lexer>(new Lexer());
	parse = shared_ptr<SyntaxParse>(new SyntaxParse());
	svm = shared_ptr<SVM>(new SVM());
	env = shared_ptr<Environment>(new Environment());
	
	NativeFunc::Register(env, svm);
}

void SState::Run(string code){
	lexer->Parse(code);
	parse->Parse(*lexer);
	parse->Compile(env, svm);
	if (sc) showCode();
	svm->Run();
}

void SState::ShowCode(bool t){
	sc = t;
}

void SState::Register(RegisterFunction func[]){
	for (int i = 0;; ++i){
		string name = func[i].name;
		if (name == "") break;

		SValue sv;
		sv.sfunc = func[i].f;
		Value function(EValueType::ENATIVEFUNC, sv);
		int idx = svm->AddGlobal(function);
		SymbolInfo si = { function, idx };
		env->SetSymbol(name, si);
	}
}

void SState::showCode() const{
	printf("%s\n", svm->ShowCode().c_str());
}

SABER_NAMESPACE_END