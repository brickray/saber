#include "preprocessor.h"

SABER_NAMESPACE_BEGIN

string Preprocessor::Process(string& code){
	string ret;
	ret = comments(code);

	return ret;
}

string Preprocessor::comments(string& code){
	int size = code.size();

	string ret;
	ret.reserve(size);
	for (int i = 0; i < size; ++i){
		char c = code[i];
		if (c == '/'){
			if (code[i + 1] == '/'){
				for (int j = i + 2; j < size; ++j){
					if (code[j] == '\n' || j == size - 1){
						i = j;
						break;
					}
				}

				continue;
			}
			else if (code[i + 1] == '*'){
				for (int j = i + 2; j < size; ++j){
					if (code[j] == '*' && code[j + 1] == '/'){
						i = j + 2;
						break;
					}
				}

				continue;
			}
		}

		ret.push_back(c);
	}

	return ret;
}

SABER_NAMESPACE_END