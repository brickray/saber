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
#include "ast\astReturn.h"
#include "ast\astLocal.h"
#include "ast\astGlobal.h"
#include "ast\astProgram.h"
#include "ast\astClosure.h"

//----------------grammar------------------
// number     : '0' .. '9' + | number (.) '0' .. '9' +
// identifier : ('a' .. 'z' | 'A' .. 'Z') +
// string     : '"' ascii + '"' //ascii代表所有字符
// primary    : number | identifier | string | true | false
// term       : func | primary | (andorexpr)
// negexpr    : term | - term
// muldivexpr : negexpr ( * | / | *= | /= negexpr)*
// addsubexpr : muldivexpr ( + | - | += | -= muldivexpr)*
// compexpr   : addsubexpr ( == | != | > | >= | < | <= addsubexpr)*
// andorexpr  : compexpr ( && | || compexpr)* 
// assignexpr : identifier ( = (andorexpr | closure)) | andorexpr
// expr       : assignexpr
// block      : statement | statement block | empty
// if         : 'if' expr 'then' block (('elif' expr 'then' block) * | ('else' block)) 'end'
// while      : 'while' expr 'do' blcok 'end'
// for        : 'for' expr ',' expr (',' expr) 'do' block 'end'
// def        : 'def' identifier '(' identifier *')' block ('return' (expr | closure)) 'end'
// func       : identifier '(' primary ')' 
// closure    : 'def' '(' identifier *')' block ('return' (expr | closure)) 'end'
// statement  : if
//			  | while
//            | for
//            | def
//            | func
//            | closure
//            | expr 
// program    : statement

SABER_NAMESPACE_BEGIN

class SyntaxParse{
private:
	Lexer lexer;
	shared_ptr<Astree> astProgram;

public:
	SyntaxParse(){ astProgram = shared_ptr<Astree>(new AstProgram()); }

	void Parse(Lexer& lexer);
	void Compile(shared_ptr<Environment>& e, shared_ptr<SVM>& svm);

private:
	bool match(string name, Token** tok = nullptr);
	bool matchBreak(shared_ptr<Astree>& astree);
	bool matchContinue(shared_ptr<Astree>& astree);
	bool matchReturn(shared_ptr<Astree>& astree);
	bool matchNumber(shared_ptr<Astree>& astree);
	bool matchIdentifier(shared_ptr<Astree>& astree);
	bool matchString(shared_ptr<Astree>& astree);
	bool matchPrimary(shared_ptr<Astree>& astree);
	bool matchTerm(shared_ptr<Astree>& astree);
	bool matchNegExpr(shared_ptr<Astree>& astree);
	bool matchMuldivExpr(shared_ptr<Astree>& astree);
	bool matchAddsubExpr(shared_ptr<Astree>& astree);
	bool matchCompExpr(shared_ptr<Astree>& astree);
	bool matchAndorExpr(shared_ptr<Astree>& astree);
	bool matchAssignExpr(shared_ptr<Astree>& astree);
	bool matchExpr(shared_ptr<Astree>& astree);
	bool matchIf(shared_ptr<Astree>& astree);
	bool matchWhile(shared_ptr<Astree>& astree);
	bool matchFor(shared_ptr<Astree>& astree);
	bool matchDef(shared_ptr<Astree>& astree);
	bool matchFunc(shared_ptr<Astree>& astree);
	bool matchClosure(shared_ptr<Astree>& astree);
	bool matchStatement(shared_ptr<Astree>& astree);
};

SABER_NAMESPACE_END

#endif