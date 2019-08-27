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

void SState::Run(string code){
	string afterProcess = preprocessor->Process(code);
	lexer->Parse(afterProcess);
	parse->Parse(*lexer);
	parse->Compile(env, svm);

	svm->Run();
	if (sc) ShowCode();
}

void SState::ShowCode(bool t){
	sc = t;
}

void SState::ShowCode() const{
	printf("%s\n", svm->ShowCode().c_str());
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