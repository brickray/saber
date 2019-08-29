#ifndef H_SSTATE_H
#define H_SSTATE_H

#include "preprocessor.h"
#include "lexer.h"
#include "parse.h"
#include "svm.h"
#include "nativeFunc.h"

SABER_NAMESPACE_BEGIN

class SState{
private:
	shared_ptr<Preprocessor> preprocessor;
	shared_ptr<Lexer> lexer;
	shared_ptr<SyntaxParse> parse;
	shared_ptr<SVM> svm;
	shared_ptr<Environment> env;

	bool sc = false;

public:
	SState(){}

	void Init();
	void Compile(string code);
	void Run();
	void ShowCode(bool t);
	void ShowCode() const;

	void PushBool(bool b);
	void PushInt(int i);
	void PushFloat(float f);
	void PushString(string s);
	void PushFunction(string str);
	void CallScript(int nps);
	Value PopStack();

	void Register(RegisterFunction func[]);
	void RegisterLib(string name, RegisterFunction func[]);

	shared_ptr<Lexer> GetLexer() const { return lexer; }
	shared_ptr<SyntaxParse> GetParser() const { return parse; }
	shared_ptr<SVM> GetSVM() const { return svm; }
	shared_ptr<Environment> GetEnvironment() const { return env; }
};

SABER_NAMESPACE_END

#endif