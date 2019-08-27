#include "lexer.h"
#include "error.h"

SABER_NAMESPACE_BEGIN

Lexer::Lexer(){
	init();
}

void Lexer::Parse(string c){
	code = c;
	init();

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

void Lexer::init(){
	ptr = 0;
	lineNo = 0;
	tokenPtr = 0;
	tokens.clear();
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
				if (c == '\\'){
					char next = line[++p];
					switch (next){
					case 'a':
						tok += '\a';
						break;
					case 'b':
						tok += '\b';
						break;
					case 'f':
						tok += '\f';
						break;
					case 'n':
						tok += '\n';
						break;
					case 'r':
						tok += '\r';
						break;
					case 't':
						tok += '\t';
						break;
					case 'v':
						tok += '\v';
						break;
					case '\\':
						tok += '\\';
						break;
					case '?':
						tok += '\?';
						break;
					default:
						tok += c;
						tok += next;
					}
				}
				else{
					tok += c;
				}
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
		else if (c == '(' || c == '{'){
			tok += c;
			c = line[++p];
			type = ETokenType::ELEFT_BRACKET;
		}
		else if (c == ')' || c == '}'){
			tok += c;
			c = line[++p];
			type = ETokenType::ERIGHT_BRACKET;
		}
		else if (c == ','){
			tok += c;
			c = line[++p];
			type = ETokenType::ECOMMA;
		}
		else if (c == '.'){
			tok += c;
			c = line[++p];
			type = ETokenType::EDOT;
		}
		else if (c == '\0' || c == '\t'){
			if (p < size) c = line[++p];

			continue;
		}
		else{
			Error::GetInstance()->ProcessError("行数[%d], 错误的标识符", lineNo);
			return;
		}

		Token token(type, lineNo, tok);
		tokens.push_back(token);
	}
}

SABER_NAMESPACE_END