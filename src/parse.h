#ifndef H_PARSE_H
#define H_PARSE_H

#include "common.h"
#include "lexer.h"
#include "astree.h"
#include "ast\astPrimary.h"
#include "ast\astWhile.h"
#include "ast\astFor.h"
#include "ast\astIf.h"
#include "ast\astElif.h"
#include "ast\astStatement.h"
#include "ast\astOperator.h"
#include "ast\astDef.h"
#include "ast\astFunc.h"
#include "ast\astBreak.h"
#include "ast\astContinue.h"

//----------------grammar------------------
// number     : '0' .. '9' + | number (.) '0' .. '9' +
// identifier : ('a' .. 'z' | 'A' .. 'Z') +
// string     : '"' ascii + '"' //ascii代表所有字符
// primary    : number | identifier | string | true | false
// op         : '+' | '-'
// expr       : func | primary | primary op expr 
// block      : statement | statement block | empty
// if         : 'if' expr 'then' block (('elif' expr 'then' block) * | ('else' block)) 'end'
// while      : 'while' expr 'do' blcok 'end'
// for        : 'for' expr expr (expr) 'do' block 'end'
// def        : 'def' identifier '(' identifier *')' block ('return' (primary)) 'end'
// func       : identifier '(' primary ')' 
// statement  : if
//			  | while
//            | for
//            | def
//            | func
//            | expr 
// program    : statement

SABER_NAMESPACE_BEGIN

class SyntaxParse{
private:
	Lexer lexer;
	vector<shared_ptr<Astree>> asts;

public:
	SyntaxParse(){}

	void Parse(Lexer& lexer);
	void Compile(shared_ptr<Environment>& e, shared_ptr<SVM>& svm);

private:
	bool match(string name, Token** tok = nullptr);
	bool matchBreak(shared_ptr<Astree>& astree);
	bool matchContinue(shared_ptr<Astree>& astree);
	bool matchNumber(shared_ptr<Astree>& astree);
	bool matchIdentifier(shared_ptr<Astree>& astree);
	bool matchString(shared_ptr<Astree>& astree);
	bool matchPrimary(shared_ptr<Astree>& astree);
	bool matchOp(shared_ptr<Astree>& astree);
	bool matchExpr(shared_ptr<Astree>& astree);
	bool matchIf(shared_ptr<Astree>& astree);
	bool matchWhile(shared_ptr<Astree>& astree);
	bool matchFor(shared_ptr<Astree>& astree);
	bool matchDef(shared_ptr<Astree>& astree);
	bool matchFunc(shared_ptr<Astree>& astree);
	bool matchStatement(shared_ptr<Astree>& astree);
};

SABER_NAMESPACE_END

#endif