#include <stdio.h>
#include <fstream>
#include <sstream>
#include "lexer.h"
#include "parse.h"
#include "nativeFunc.h"

void main(){
	ifstream stream;
	stream.open("../test/script.sa");
	if (!stream.is_open()){
		printf("unable to open file\n");
		return;
	}
	stringstream buffer;
	buffer << stream.rdbuf();
	string code(buffer.str());

	shared_ptr<Environment> env = shared_ptr<Environment>(new Environment());
	NativeFunc::Register(env);
	Lexer lexer(code);
	SyntaxParse parse(lexer);
	parse.Parse();
	shared_ptr<VM> vm = shared_ptr<VM>(new VM());
	parse.Eval(env, vm);
	vm->Run();
	printf("%s\n", vm->ShowCode().c_str());

	getchar();
}