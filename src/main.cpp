#include <stdio.h>
#include <fstream>
#include <sstream>
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
	state.Run(code);

	getchar();
}