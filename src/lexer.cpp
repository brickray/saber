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
	lineNo = 1;
	tokenPtr = 0;
	tokens.clear();
}

void Lexer::parse(){
	int p = 0;
	char c = code[p];
	int size = code.length();
	bool comments = true;

	while (p < size){
		string tok;
		ETokenType type;
		while (isSpace(c)){
			c = code[++p];
		}
		if (isDigit(c)){
			do{
				tok += c;
				c = code[++p];
			} while (isDigit(c));
			if (c == '.'){
				do{
					tok += c;
					c = code[++p];
				} while (isDigit(c));
			}

			type = ETokenType::ENUMBER;
		}
		else if (isLetter(c) || c == '_'){
			do{
				tok += c;
				c = code[++p];
			} while (isLetter(c) || isDigit(c) || c == '_');

			if (reserved.find(tok) != reserved.end())
				type = ETokenType::ERESERVED;
			else
				type = ETokenType::EIDENTIFIER;
		}
		else if (c == '\"' || c == '\''){
			char quote = c;
			c = code[++p];
			while (c != quote){
				if (c == '\\'){
					char next = code[++p];
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
					case '\"':
						tok += '\"';
						break;
					case '\'':
						tok += '\'';
						break;
					default:
						tok += c;
						tok += next;
					}
				}
				else{
					tok += c;
				}
				c = code[++p];
			}
			c = code[++p];
			type = ETokenType::ESTRING;
		}
		else if (c == '/' && comments){
			//comments
			if (code[p + 1] == '/'){
				for (int j = p + 2; j < size; ++j){
					if (code[j] == '\n'){
						p = j - 1;
						break;
					}
					else if (j == size - 1){
						p = j;
						break;
					}
				}
			}
			else if (code[p + 1] == '*'){
				for (int j = p + 2; j < size; ++j){
					if (code[j] == '*' && code[j + 1] == '/'){
						p = j + 2;
						break;
					}
				}
			}
			else{
				comments = false;
				continue;
			}

			c = code[++p];
			continue;
		}
		else if (c == '=' || c == '!' || c == '+' || c == '-' ||
			c == '*' || c == '/' || c == '%' ||
			c == '<' || c == '>'){
			if (c == '/') comments = true;
			tok += c;
			c = code[++p];
			if (c == '='){
				tok += c;
				c = code[++p];
			}
			type = ETokenType::EOPERATOR;
		}
		else if (c == '|'){
			tok += c;
			c = code[++p];
			if (c == '|'){
				tok += c;
				c = code[++p];
			}
			type = ETokenType::EOPERATOR;
		}
		else if (c == '&'){
			tok += c;
			c = code[++p];
			if (c == '&'){
				tok += c;
				c = code[++p];
			}
			type = ETokenType::EOPERATOR;
		}
		else if (c == ';'){
			c = code[++p];
			continue;
		}
		else if (c == '(' || c == '{' || c == '['){
			tok += c;
			c = code[++p];
			type = ETokenType::ELEFT_BRACKET;
		}
		else if (c == ')' || c == '}' || c == ']'){
			tok += c;
			c = code[++p];
			type = ETokenType::ERIGHT_BRACKET;
		}
		else if (c == ','){
			tok += c;
			c = code[++p];
			type = ETokenType::ECOMMA;
		}
		else if (c == '.'){
			tok += c;
			c = code[++p];
			if (c == '.'){
				tok += c;
				c = code[++p];
				if (c == '.'){
					tok += c;
					c = code[++p];
					type = ETokenType::ETDOT;
				}
				else{
					Error::GetInstance()->ProcessError("行数[%d], 错误的标识符[..]", lineNo);
				}
			}
			else{
				type = ETokenType::EDOT;
			}
		}
		else if (c == '#'){
			tok += c;
			c = code[++p];
			type = ETokenType::EHASH;
		}
		else if (c == '\n'){
			lineNo++;
			c = code[++p];
			continue;
		}
		else if (c == '\0' || c == '\t'){
			if (p < size) c = code[++p];

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