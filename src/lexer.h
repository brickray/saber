#ifndef H_LEXER_H
#define H_LEXER_H

#include "common.h"

SABER_NAMESPACE_BEGIN

enum class ETokenType{
	ERESERVED,
	EIDENTIFIER,
	ENUMBER,
	ESTRING,
	EOPERATOR,
	ESEMICOLON,
	ELEFT_BRACKET,
	ERIGHT_BRACKET,
	ECOMMA,
	EDOT,
	ENUM,
};

static string tokenTypeString[int(ETokenType::ENUM)] = {
	"reserved",
	"identifier",
	"number",
	"string",
	"operator",
	"semicolon",
	"left bracket",
	"right bracket",
	"comma",
};

static hash_set<string> reserved = {
	"if",
	"then",
	"else",
	"elif",
	"end",
	"and",
	"or",
	"while",
	"do",
	"break",
	"continue",
	"def",
	"return",
	"local",
};

class Token{
private:
	ETokenType type;
	int lineNumber;
	string token;

public:
	Token(ETokenType t, int l, string s) :type(t), lineNumber(l), token(s){}

	int GetLineNumber() const { return lineNumber; }
	ETokenType GetTokenType() const { return type; }
	void SetTokenType(ETokenType t) { type = t; }
	string GetToken() const { return token; }
	string ToString() const{
		string ret;
		ret += "token = [" + token + "]";
		ret += ", type = [" + tokenTypeString[int(type)] + "]";
		ret += ", line = [" + to_string(lineNumber) + "]";
		return ret;
	}
};

class Lexer{
private:
	string code;
	int ptr;
	int lineNo;

	vector<Token> tokens;
	int tokenPtr;

public:
	Lexer();

	void Parse(string c);
	vector<Token> GetTokens() const { return tokens; }
	Token* NextToken();
	Token* PrevToken();
	void Back();
	bool IsEnd() const { return tokenPtr >= tokens.size(); }

private:
	void init();
	void parse();
	string readLine();
	void parseLine(string line);

	bool isLetter(char c) const { return ('A' <= c && c <= 'Z') || ('a' <= c && c <= 'z'); }
	bool isDigit(char c) const { return '0' <= c && c <= '9'; }
	bool isSpace(char c) const { return c == ' '; }
};

SABER_NAMESPACE_END

#endif