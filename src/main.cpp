#include <stdio.h>
#include <fstream>
#include <sstream>
#include <time.h>
#include "sstate.h"

SABER_NAMESPACE_USING

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
	
	SState state;
	state.ShowCode(true);
	state.Init();
	state.Compile(code);
	state.Run();

	state.PushInt(10);
	state.PushInt(2);
	state.PushFloat(0.5);
	for (int i = 0; i < 100; ++i){
		state.PushInt(5);
	}
	state.PushFunction("add");
	state.CallScript(103);
	Value ret = state.PopStack();
	printf("%s\n", ret.ToString().c_str());

	getchar();
}