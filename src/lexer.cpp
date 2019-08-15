#include "lexer.h"

Lexer::Lexer(){
	ptr = 0;
	lineNo = 0;
	tokenPtr = 0;
}

void Lexer::Parse(string c){
	code = c;

	parse();
}

Token* Lexer::NextToken(){
	if (tokenPtr >= tokens.size()) return nullptr;

	return &tokens[tokenPtr++];
}

Token* Lexer::PrevToken(){
	if (tokenPtr - 2 < 0) return nullptr;

	return &tokens[tokenPtr - 2];
}

void Lexer::Back(){
	--tokenPtr;
}

void Lexer::parse(){
	while (true){
		string line = readLine();
		if (line == ""){
			if (ptr < code.size()) continue;
			else break;
		}

		parseLine(line);
	}
}

string Lexer::readLine(){
	int p = code.find('\n', ptr);
	if (p == -1){
		if (ptr < code.size()){
			string line = code.substr(ptr, code.size() - ptr);
			ptr = code.size();
			return line;
		}

		return "";
	}

	string line = code.substr(ptr, p - ptr);
	ptr += p - ptr + 1;
	lineNo++;
	return line;
}

void Lexer::parseLine(string line){
	int p = 0;
	char c = line[p];
	int size = line.length();

	while (p < size){
		string tok;
		ETokenType type;
		while (isSpace(c)){
			c = line[++p];
		}
		if (isDigit(c)){
			do{
				tok += c;
				c = line[++p];
			} while (isDigit(c));
			if (c == '.'){
				do{
					tok += c;
					c = line[++p];
				} while (isDigit(c));
			}
		
			type = ETokenType::ENUMBER;
		}
		else if (isLetter(c)){
			do{
				tok += c;
				c = line[++p];
			} while (isLetter(c) || isDigit(c));
		
			if (reserved.find(tok) != reserved.end())
				type = ETokenType::ERESERVED;
			else
				type = ETokenType::EIDENTIFIER;
		}
		else if (c == '\"'){
			c = line[++p];
			while (c != '\"'){
				tok += c;
				c = line[++p];
			}
			c = line[++p];
			type = ETokenType::ESTRING;
		}
		else if (c == '=' || c == '!' || c == '+' || c == '-' || 
			c == '*' || c == '/' || c == '%' ||
			c == '<' || c == '>'){
			tok += c;
			c = line[++p];
			if (c == '='){
				tok += c;
				c = line[++p];
			}
			type = ETokenType::EOPERATOR;
		}
		else if (c == '|'){
			tok += c;
			c = line[++p];
			if (c == '|'){
				tok += c;
				c = line[++p];
			}
			type = ETokenType::EOPERATOR;
		}
		else if (c == '&'){
			tok += c;
			c = line[++p];
			if (c == '&'){
				tok += c;
				c = line[++p];
			}
			type = ETokenType::EOPERATOR;
		}
		else if (c == ';'){
			c = line[++p];
			continue;
		}
		else if (c == '('){
			tok += c;
			c = line[++p];
			type = ETokenType::ELEFT_BRACKET;
		}
		else if (c == ')'){
			tok += c;
			c = line[++p];
			type = ETokenType::ERIGHT_BRACKET;
		}
		else if (c == '\0' || c == '\t'){
			if (p < size) c = line[++p];

			continue;
		}
		else{
			printf("行数[%d], 错误的标识符\n", lineNo);
			return;
		}

		Token token(type, lineNo, tok);
		tokens.push_back(token);
	}
}