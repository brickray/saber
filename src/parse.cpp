#include "parse.h"
#include "error.h"

SABER_NAMESPACE_BEGIN

shared_ptr<Astree> RotateBTree(shared_ptr<Astree>& root){
	if (!root || !root->GetNumChildren()) return root;

	bool first = true;
	shared_ptr<Astree> node = root, ret = root, temp;
	while (node->GetNumChildren() &&  node->GetToken()){
		shared_ptr<Astree> left = node->GetChild(0);
		shared_ptr<Astree> right = node->GetChild(1);
		node->RemoveAllChild();
		if (!first){
			ret->AddChild(left);
			temp = ret;
			ret = node;
			ret->AddChild(temp);
		}
		else{
			ret->AddChild(left);
		}

		node = right;
		temp = right;

		first = false;
	}

	ret->AddChild(temp);
	return ret;
}

void SyntaxParse::Parse(Lexer& lex){
	astProgram = shared_ptr<Astree>(new AstProgram());
	lexer = lex;

	int idx = 0;
	while (!lexer.IsEnd()){
		shared_ptr<Astree> node = shared_ptr<Astree>(new AstStatement());
		if (matchStatement(node)){
			astProgram->AddChild(node);
		}
	}
}

void SyntaxParse::Compile(shared_ptr<Environment>& e, shared_ptr<SVM>& svm, BlockCnt& bc){
	astProgram->Compile(e, svm, bc);
}

bool SyntaxParse::match(string name, Token** tok){
	Token* t = lexer.NextToken();
	if (!t) return false;
	if (t->GetToken() == name){
		if (tok) *tok = t;
		return true;
	}

	lexer.Back();
	return false;
}

bool SyntaxParse::matchBreak(shared_ptr<Astree>& astree){
	shared_ptr<Astree> astBreak = shared_ptr<Astree>(new AstBreak());
	Token* tok;
	if (match("break", &tok)){
		astBreak->SetToken(tok);
		astree->AddChild(astBreak);

		return true;
	}

	return false;
}

bool SyntaxParse::matchContinue(shared_ptr<Astree>& astree){
	shared_ptr<Astree> astContinue = shared_ptr<Astree>(new AstContinue());
	Token* tok;
	if (match("continue", &tok)){
		astContinue->SetToken(tok);
		astree->AddChild(astContinue);

		return true;
	}

	return false;
}

bool SyntaxParse::matchReturn(shared_ptr<Astree>& astree){
	shared_ptr<Astree> astReturn = shared_ptr<Astree>(new AstReturn());
	Token* tok;
	if (match("return", &tok)){
		astReturn->SetToken(tok);
		shared_ptr<Astree> stat = shared_ptr<Astree>(new AstStatement());
		if (matchExpr(stat)){
			astReturn->AddChild(stat);
			dynamic_cast<AstReturn*>(astReturn.get())->SetNumRetParams(1);
			dynamic_cast<AstReturn*>(astReturn.get())->SetMaybeTailCall();
		}
		else if (matchClosure(stat)){
			astReturn->AddChild(stat);
			dynamic_cast<AstReturn*>(astReturn.get())->SetNumRetParams(1);
		}
		else if (matchTable(stat)){
			astReturn->AddChild(stat);
			dynamic_cast<AstReturn*>(astReturn.get())->SetNumRetParams(1);
		}
		astree->AddChild(astReturn);

		return true;
	}

	return false;
}

bool SyntaxParse::matchNumber(shared_ptr<Astree>& astree){
	Token* tok = lexer.NextToken();
	if (!tok) return false;
	if (tok->GetTokenType() == ETokenType::ENUMBER){
		astree->SetToken(tok);
		return true;
	}

	lexer.Back();
	return false;
}

bool SyntaxParse::matchIdentifier(shared_ptr<Astree>& astree){
	Token* tok = lexer.NextToken();
	if (!tok) return false;
	if (tok->GetTokenType() == ETokenType::EIDENTIFIER){
		astree->SetToken(tok);
		return true;
	}

	lexer.Back();
	return false;
}

bool SyntaxParse::matchString(shared_ptr<Astree>& astree){
	Token* tok = lexer.NextToken();
	if (!tok) return false;
	if (tok->GetTokenType() == ETokenType::ESTRING){
		astree->SetToken(tok);
		return true;
	}

	lexer.Back();
	return false;
}

bool SyntaxParse::matchPrimary(shared_ptr<Astree>& astree){
	if (matchNumber(astree)) return true;
	if (matchString(astree)) return true;

	return false;
}

bool SyntaxParse::matchLValue(shared_ptr<Astree>& astree, bool array){
	shared_ptr<Astree> name = shared_ptr<Astree>(new AstPrimary());
	if (!array){
		if (!matchIdentifier(name)) return false;
	}
	bool d = astree->GetToken() ? (astree->GetToken()->GetToken() == "." ||
		astree->GetToken()->GetToken() == "[") : false;
	if (d) name->SetTable(true);
	Token* tok;
	if (match(".", &tok)){
		shared_ptr<Astree> dot = shared_ptr<Astree>(new AstDot());
		dot->SetToken(tok);
		if (!array) dot->AddChild(name);
		if (matchLValue(dot)){
			if (!array){
				if (d) astree->AddChild(dot);
				else astree = dot;
			}
			else{
				astree->AddChild(dot->GetChild(0));
			}

			return true;
		}

		return false;
	}
	else if (match("[", &tok)){
		shared_ptr<Astree> dot = shared_ptr<Astree>(new AstDot());
		shared_ptr<Astree> stat = shared_ptr<Astree>(new AstStatement());
		dot->SetToken(tok);
		if(!array) dot->AddChild(name);
		if (matchExpr(stat)){
			if (!match("]")){
				Error::GetInstance()->ProcessError("行数:%d, table语法错误，缺少']'", tok->GetLineNumber());
				return false;
			}

			shared_ptr<Astree> ndot = shared_ptr<Astree>(new AstDot());
			ndot->SetToken(tok);
			ndot->AddChild(stat);
			if (matchLValue(ndot, true)){
				if (dot->GetNumChildren() == 0){
					astree->AddChild(ndot);
					return true;
				}
				dot->AddChild(ndot);
			}
			else{
				if (dot->GetNumChildren() == 0){
					astree->AddChild(stat);
					return true;
				}

				dot->AddChild(stat);
			}

			if (d) astree->AddChild(dot);
			else astree = dot;

			return true;
		}

		return false;
	}

	if (!array){
		if (d) astree->AddChild(name);
		else astree = name;
		return true;
	}

	return false;
}

bool SyntaxParse::matchTerm(shared_ptr<Astree>& astree){
	if (matchFunc(astree)) return true;
	shared_ptr<Astree> left = shared_ptr<Astree>(new AstPrimary());
	if (matchPrimary(left)) {
		astree = left;
		return true;
	}
	bool hash = false;
	shared_ptr<Astree> astHash = shared_ptr<Astree>(new AstHash());
	if (match("#")){
		astree = astHash;
		hash = true;
		shared_ptr<Astree> p = shared_ptr<Astree>(new AstPrimary());
		if (matchPrimary(p)){
			Error::GetInstance()->ProcessError("行数:%d,[#]只能应用在左值上", p->GetToken()->GetLineNumber());
			return false;
		}
	}

	if (matchLValue(left)){
		if (hash) astree->AddChild(left);
		else astree = RotateBTree(left);
		return true;
	}
	Token* tok;
	if (match("(", &tok)){
		if (!matchAndorExpr(astree)) return false;
		if (!match(")")){
			Error::GetInstance()->ProcessError("行数:%d, 表达式语法错误,缺少')'", tok->GetLineNumber());
			return false;
		}

		return true;
	}

	return false;
}

bool SyntaxParse::matchNegExpr(shared_ptr<Astree>& astree){
	Token* tok;
	if (match("-", &tok)){
		if (tok->GetTokenType() == ETokenType::EOPERATOR){
			shared_ptr<Astree> stat = shared_ptr<Astree>(new AstStatement());
			shared_ptr<Astree> op = shared_ptr <Astree>(new AstOperator());
			op->SetToken(tok);
			if (!matchTerm(stat)) return false;
			op->AddChild(stat);
			astree->AddChild(op);

			return true;
		}

		lexer.Back();
	}

	return matchTerm(astree);
}

bool SyntaxParse::matchMuldivExpr(shared_ptr<Astree>& astree){
	shared_ptr<Astree> stat = shared_ptr<Astree>(new AstStatement());
	if (!matchNegExpr(stat)) return false;
	Token* tok;
	shared_ptr<Astree> op = shared_ptr<Astree>(new AstOperator());
	if (match("*", &tok) || match("/", &tok) || 
		match("*=", &tok) || match("/=", &tok) ||
		match("%", &tok) || match("%=", &tok)){
		if (tok->GetTokenType() == ETokenType::EOPERATOR){
			if (tok->GetToken() == "*=" || tok->GetToken() == "/=" || tok->GetToken() == "%="){
				if (!stat->GetToken() || stat->GetToken()->GetTokenType() != ETokenType::EIDENTIFIER){
					Error::GetInstance()->ProcessError("行数:%d, %s : 左操作数必须为左值", tok->GetLineNumber(), tok->GetToken().c_str());
					return false;
				}
			}
			op->SetToken(tok);
			op->AddChild(stat);
			if (!matchMuldivExpr(op)) return false;
			astree->AddChild(op);
			return true;
		}

		lexer.Back();
	}

	astree->AddChild(stat);
	return true;
}

bool SyntaxParse::matchAddsubExpr(shared_ptr<Astree>& astree){
	shared_ptr<Astree> stat = shared_ptr<Astree>(new AstStatement());
	if (!matchMuldivExpr(stat)) return false;
	Token* tok;
	shared_ptr<Astree> op = shared_ptr<Astree>(new AstOperator());
	if (match("+", &tok) || match("-", &tok) || match("+=", &tok) || match("-=", &tok)){
		if (tok->GetTokenType() == ETokenType::EOPERATOR){
			if (tok->GetToken() == "+=" || tok->GetToken() == "-="){
				if (!stat->GetChild(0)->GetToken() || stat->GetChild(0)->GetToken()->GetTokenType() != ETokenType::EIDENTIFIER){
					Error::GetInstance()->ProcessError("行数:%d, %s : 左操作数必须为左值", tok->GetLineNumber(), tok->GetToken().c_str());
					return false;
				}
			}
			op->SetToken(tok);
			op->AddChild(stat);
			if (!matchAddsubExpr(op)) return false;
			astree->AddChild(op);
			return true;
		}

		lexer.Back();
	}

	astree->AddChild(stat);
	return true;
}

bool SyntaxParse::matchCompExpr(shared_ptr<Astree>& astree){
	shared_ptr<Astree> stat = shared_ptr<Astree>(new AstStatement());
	if (!matchAddsubExpr(stat)) return false;
	Token* tok;
	shared_ptr<Astree> op = shared_ptr<Astree>(new AstOperator());
	if (match("==", &tok) || match("!=", &tok) || 
		match(">", &tok) || match("<", &tok) ||
		match(">=", &tok) || match("<=", &tok)){
		if (tok->GetTokenType() == ETokenType::EOPERATOR){
			op->SetToken(tok);
			op->AddChild(stat);
			if (!matchCompExpr(op)) return false;
			astree->AddChild(op);
			return true;
		}

		lexer.Back();
	}

	astree->AddChild(stat);
	return true;
}

bool SyntaxParse::matchAndorExpr(shared_ptr<Astree>& astree){
	shared_ptr<Astree> stat = shared_ptr<Astree>(new AstStatement());
	if (!matchCompExpr(stat)) return false;
	Token* tok;
	shared_ptr<Astree> op = shared_ptr<Astree>(new AstOperator());
	if (match("&&", &tok) || match("||", &tok)){
		if (tok->GetTokenType() == ETokenType::EOPERATOR){
			op->SetToken(tok);
			op->AddChild(stat);
			if (!matchAndorExpr(op)) return false;
			astree->AddChild(op);
			return true;
		}

		lexer.Back();
	}

	astree->AddChild(stat);
	return true;
}

bool SyntaxParse::matchAssignExpr(shared_ptr<Astree>& astree){
	shared_ptr<Astree> l = shared_ptr<Astree>(new AstLocal());
	Token* local;
	if (match("local", &local)){
		l->SetToken(local);
		shared_ptr<Astree> name = shared_ptr<Astree>(new AstPrimary());
		if (matchLValue(name)){
			l->AddChild(name);
			Token* tok;
			shared_ptr<Astree> op = shared_ptr<Astree>(new AstOperator());
			if (match("=", &tok)){
				op->SetToken(tok);
				op->AddChild(l);
				shared_ptr<Astree> stat = shared_ptr<Astree>(new AstStatement());
				if (matchClosure(stat)){
					op->AddChild(stat);
					astree->AddChild(op);
					return true;
				}
				else if (matchAndorExpr(stat)){
					op->AddChild(stat);
					astree->AddChild(op);
					return true;
				}
				else if (matchTable(stat)){
					op->AddChild(stat);
					astree->AddChild(op);
					return true;
				}

				return false;
			}

			astree->AddChild(l);
			return true;
		}

		return false;
	}

	shared_ptr<Astree> name = shared_ptr<Astree>(new AstPrimary());
	int p = lexer.GetTkptr();
	if (matchLValue(name)){
		Token* tok;
		if (match("=", &tok)){
			bool istable = name->GetToken()->GetToken() == "." ||
				name->GetToken()->GetToken() == "[";
			name = RotateBTree(name);
			shared_ptr<Astree> g = shared_ptr<Astree>(new AstGlobal());
			g->SetTable(istable);
			g->AddChild(name);
			shared_ptr<Astree> op = shared_ptr<Astree>(new AstOperator());
			op->SetTable(istable);
			op->SetToken(tok);
			op->AddChild(g);
			shared_ptr<Astree> stat = shared_ptr<Astree>(new AstStatement());
			if (matchClosure(stat)){
				op->AddChild(stat);
				astree->AddChild(op);
				return true;
			}
			else if (matchAndorExpr(stat)){
				op->AddChild(stat);
				astree->AddChild(op);
				return true;
			}
			else if (matchTable(stat)){
				op->AddChild(stat);
				astree->AddChild(op);
				return true;
			}

			return false;
		}

		lexer.SetTkptr(p);
	}

	return matchAndorExpr(astree);
}

bool SyntaxParse::matchExpr(shared_ptr<Astree>& astree){
	return matchAssignExpr(astree);
}

bool SyntaxParse::matchIf(shared_ptr<Astree>& astree){
	shared_ptr<Astree> astIf = shared_ptr<Astree>(new AstIf());
	Token* iftok;
	if (!match("if", &iftok)) return false;
	astree->AddChild(astIf);
	shared_ptr<Astree> expr = shared_ptr<Astree>(new AstStatement());
	if (!matchExpr(expr)) return false;
	astIf->AddChild(expr);
	if (!match("then")){
		Error::GetInstance()->ProcessError("行数:%d, if语句语法错误,缺少关键字[then]", iftok->GetLineNumber());
		return false;
	}
	shared_ptr<Astree> stat = shared_ptr<Astree>(new AstStatement());
	astIf->AddChild(stat);
	while (true){
		shared_ptr<Astree> statement = shared_ptr<Astree>(new AstStatement());
		stat->AddChild(statement);
		if (!matchStatement(statement)) break;
	}

	while (match("elif")){
		shared_ptr<Astree> elif = shared_ptr<Astree>(new AstElif());
		astIf->AddChild(elif);
		shared_ptr<Astree> expr = shared_ptr<Astree>(new AstStatement());
		if (!matchExpr(expr)) return false;
		elif->AddChild(expr);
		Token* eliftok;
		if (!match("then", &eliftok)){
			Error::GetInstance()->ProcessError("行数:%d, elif语句语法错误,缺少关键字[then]", eliftok->GetLineNumber());
			return false;
		}
		shared_ptr<Astree> stat = shared_ptr<Astree>(new AstStatement());
		elif->AddChild(stat);
		while (true){
			shared_ptr<Astree> statement = shared_ptr<Astree>(new AstStatement());
			stat->AddChild(statement);
			if (!matchStatement(statement)) break;
		}
	}

	if (match("else")){
		dynamic_cast<AstIf*>(astIf.get())->SetElseBlock();
		shared_ptr<Astree> stat = shared_ptr<Astree>(new AstStatement());
		astIf->AddChild(stat);
		while (true){
			shared_ptr<Astree> statement = shared_ptr<Astree>(new AstStatement());
			stat->AddChild(statement);
			if (!matchStatement(statement)) break;
		}
	}

	if (!match("end")) return false;

	return true;
}

bool SyntaxParse::matchWhile(shared_ptr<Astree>& astree){
	shared_ptr<Astree> astWhile = shared_ptr<Astree>(new AstWhile());
	Token* tok;
	if (!match("while", &tok)) return false;
	astWhile->SetToken(tok);
	astree->AddChild(astWhile);
	shared_ptr<Astree> expr = shared_ptr<Astree>(new AstStatement());
	if (!matchExpr(expr)) return false;
	astWhile->AddChild(expr);
	if (!match("do")){
		Error::GetInstance()->ProcessError("行数:%d, while语句语法错误,缺少关键字[do]", tok->GetLineNumber());

		return false;
	}
	while (true){
		shared_ptr<Astree> statement = shared_ptr<Astree>(new AstStatement());
		astWhile->AddChild(statement);
		if (!matchStatement(statement)) break;
	}
	if (!match("end")) return false;

	return true;
}

bool SyntaxParse::matchFor(shared_ptr<Astree>& astree){
	shared_ptr<Astree> astFor = shared_ptr<Astree>(new AstFor());
	Token* tok;
	if (!match("for", &tok)) return false;
	astFor->SetToken(tok);
	astree->AddChild(astFor);
	shared_ptr<Astree> expr = shared_ptr<Astree>(new AstStatement());
	if (!matchExpr(expr)) return false;
	astFor->AddChild(expr);
	if (!match(",")){
		Error::GetInstance()->ProcessError("行数:%d, for语句语法错误,循环条件间缺少[,]", tok->GetLineNumber());
		return false;
	}
	shared_ptr<Astree> cond = shared_ptr<Astree>(new AstStatement());
	if (!matchExpr(cond)) return false;
	astFor->AddChild(cond);
	shared_ptr<Astree> step = shared_ptr<Astree>(new AstStatement());
	if (match(",")){
		//normal
		if(matchExpr(step)) astFor->AddChild(step);
		else{
			Error::GetInstance()->ProcessError("行数:%d, for语句语法错误,缺少step条件", tok->GetLineNumber());
			return false;
		}
	}
	else if (match("in")){
		//generic
		
	}

	if (!match("do")){
		Error::GetInstance()->ProcessError("行数:%d, for语句语法错误,缺少关键字[do]", tok->GetLineNumber());
		return false;
	}

	shared_ptr<Astree> stat = shared_ptr<Astree>(new AstStatement());
	while (true){
		shared_ptr<Astree> statement = shared_ptr<Astree>(new AstStatement());
		if (!matchStatement(statement)) break;
		stat->AddChild(statement);
	}
	astFor->AddChild(stat);

	if (!match("end")) return false;

	return true;
}

bool SyntaxParse::matchDef(shared_ptr<Astree>& astree){
	shared_ptr<Astree> def = shared_ptr<Astree>(new AstDef());
	Token* tok;
	if (!match("def", &tok)) return false;
	shared_ptr<Astree> fun = shared_ptr<Astree>(new AstPrimary());
	if (matchIdentifier(fun)){
		def->AddChild(fun);
	}
	else{
		if (matchPrimary(fun)){
			Error::GetInstance()->ProcessError("行数:%d, 函数定义语法错误,函数名必须以字母开头，由字母和数字组成", tok->GetLineNumber());
			return false;
		}

		lexer.Back();

		return false;
	}
	if (!match("(")){
		Error::GetInstance()->ProcessError("行数:%d, 函数定义语法错误,缺少'('", tok->GetLineNumber());
		return false;
	}

	bool first = true;
	do{
		Token* tdot;
		shared_ptr<Astree> param = shared_ptr<Astree>(new AstPrimary());
		if (matchIdentifier(param)){
			def->AddChild(param);
		}
		else if (match("...", &tdot)){
			shared_ptr<Astree> args = shared_ptr<Astree>(new AstPrimary());
			args->SetToken(tdot);
			def->AddChild(args);
			break;
		}
		else{
			if (first) break;
			else{
				Error::GetInstance()->ProcessError("行数:%d, 函数定义语法错误,','后面缺少参数", tok->GetLineNumber());
				return false;
			}
		}
		first = false;
	} while (match(","));

	AstDef* d = dynamic_cast<AstDef*>(def.get());
	d->SetNumParams(d->GetNumChildren() - 1);

	if (!match(")")){
		Error::GetInstance()->ProcessError("行数:%d, 函数定义语法错误,缺少')'", tok->GetLineNumber());
		return false;
	}

	shared_ptr<Astree> stat = shared_ptr<Astree>(new AstStatement());
	while (true){
		shared_ptr<Astree> statement = shared_ptr<Astree>(new AstStatement());
		if (!matchStatement(statement)) break;
		stat->AddChild(statement);
	}

	def->AddChild(stat);
	
	if (!match("end")) return false;

	astree->AddChild(def);

	return true;
}

bool SyntaxParse::matchFunc(shared_ptr<Astree>& astree, bool fromClosure){
	shared_ptr<Astree> func = shared_ptr<Astree>(new AstFunc());
	shared_ptr<Astree> name = shared_ptr<Astree>(new AstPrimary());
	Token* tok = nullptr;
	int p = lexer.GetTkptr();
	if (!fromClosure){
		if (!matchLValue(name)) return false;
		
		bool istable = name->GetToken()->GetToken() == "." ||
			name->GetToken()->GetToken() == "[";
		func->SetTable(istable);

		name = RotateBTree(name);
		func->AddChild(name);
	}

	if (!match("(", &tok)){
		lexer.SetTkptr(p);
		return false;
	}

	(dynamic_cast<AstFunc*>(func.get())->SetFunc(fromClosure));

	do{	
		shared_ptr<Astree> parent = shared_ptr<Astree>(new AstStatement());
		bool first = true;
		do{
			shared_ptr<Astree> param = shared_ptr<Astree>(new AstStatement());
			if (matchClosure(param)){
				parent->AddChild(param);
			}
			else if (matchExpr(param)){
				parent->AddChild(param);
			}
			else{
				if (first) break;
				else{
					Error::GetInstance()->ProcessError("行数:%d, 函数调用语法错误,','后面缺少参数", tok->GetLineNumber());
					return false;
				}
			}
			first = false;
		} while (match(","));
		func->AddChild(parent);

		if (!match(")")){
			Error::GetInstance()->ProcessError("行数:%d, 函数调用语法错误,缺少')'", tok->GetLineNumber());
			return false;
		}
	} while (match("(", &tok));

	astree->AddChild(func);
	shared_ptr<Astree> t = shared_ptr<Astree>(new AstDot());
	if (matchLValue(t, true)){
		(dynamic_cast<AstDot*>(t.get()))->SetFunc(true);
		shared_ptr<Astree> c = t->GetChild(0);
		while (c){
			AstDot* dot = dynamic_cast<AstDot*>(c.get());
			if (dot) dot->SetFunc(true);
			if (c->GetNumChildren() > 1)
				c = c->GetChild(1);
			else
				c = nullptr;
		}
		astree->AddChild(t);
		matchFunc(astree, true);
	}

	return true;
}

bool SyntaxParse::matchClosure(shared_ptr<Astree>& astree){
	shared_ptr<Astree> closure = shared_ptr<Astree>(new AstClosure());
	int p = lexer.GetTkptr();
	int numParentheses = 0;
	while (match("(")) numParentheses++;
	Token* tok;
	if (!match("def", &tok)){
		lexer.SetTkptr(p);
		return false;
	}
	if (!match("(")){
		Error::GetInstance()->ProcessError("行数:%d, 函数定义语法错误,缺少'('", tok->GetLineNumber());
		return false;
	}

	bool first = true;
	do{
		Token* tdot;
		shared_ptr<Astree> param = shared_ptr<Astree>(new AstPrimary());
		if (matchIdentifier(param)){
			closure->AddChild(param);
		}
		else if (match("...", &tdot)){
			shared_ptr<Astree> args = shared_ptr<Astree>(new AstPrimary());
			args->SetToken(tdot);
			closure->AddChild(args);
			break;
		}
		else{
			if (first) break;
			else{
				Error::GetInstance()->ProcessError("行数:%d, 函数定义语法错误,','后面缺少参数", tok->GetLineNumber());
				return false;
			}
		}
		first = false;
	} while (match(","));

	AstClosure* d = dynamic_cast<AstClosure*>(closure.get());
	d->SetNumParams(d->GetNumChildren());

	if (!match(")")){
		Error::GetInstance()->ProcessError("行数:%d, 函数定义语法错误,缺少')'", tok->GetLineNumber());
		return false;
	}

	shared_ptr<Astree> stat = shared_ptr<Astree>(new AstStatement());
	while (true){
		shared_ptr<Astree> statement = shared_ptr<Astree>(new AstStatement());
		if (!matchStatement(statement)) break;
		stat->AddChild(statement);
	}

	closure->AddChild(stat);

	if (!match("end")) return false;

	astree->AddChild(closure);

	int num = numParentheses;
	while (num){
		if (!match(")")) break;
		num--;
	}
	if (matchFunc(astree, true) && (numParentheses == 0 || num != 0)){
		Error::GetInstance()->ProcessError("行数:%d, 匿名函数调用语法错误,缺少')'", tok->GetLineNumber());
		return false;
	}

	return true;
}

bool SyntaxParse::matchTableInit(shared_ptr<Astree>& astree){
	shared_ptr<Astree> name = shared_ptr<Astree>(new AstPrimary());
	if (!matchString(name)) return false;
	Token* tok;
	if (match("=", &tok)){
		shared_ptr<Astree> op = shared_ptr<Astree>(new AstOperator());
		op->SetTable(true);
		dynamic_cast<AstOperator*>(op.get())->SetTableInit();
		op->SetToken(tok);
		op->AddChild(name);
		shared_ptr<Astree> stat = shared_ptr<Astree>(new AstStatement());
		if (matchClosure(stat)){
			op->AddChild(stat);
			astree->AddChild(op);
			return true;
		}
		else if (matchAndorExpr(stat)){
			op->AddChild(stat);
			astree->AddChild(op);
			return true;
		}
		else if (matchTable(stat)){
			op->AddChild(stat);
			astree->AddChild(op);
			return true;
		}

		return false;
	}

	return true;
}

bool SyntaxParse::matchTable(shared_ptr<Astree>& astree){
	Token* tok;
	if (match("{", &tok)){
		shared_ptr<Astree> t = shared_ptr<Astree>(new AstTable());
		astree->AddChild(t);
		bool first = true;
		do{
			shared_ptr<Astree> ast = shared_ptr<Astree>(new AstStatement());
			if (matchTableInit(ast)){
				t->AddChild(ast);
			}
			else{
				if (first) break;
				else{
					Error::GetInstance()->ProcessError("行数:%d, 函数调用语法错误,','后面缺少参数", tok->GetLineNumber());
					return false;
				}
			}
			first = false;
		} while (match(","));

		if (!match("}")){
			Error::GetInstance()->ProcessError("行数:%d, 表语法错误缺少}", tok->GetLineNumber());
			return false;
		}

		return true;
	}

	return false;
}

bool SyntaxParse::matchStatement(shared_ptr<Astree>& astree){
	if (matchIf(astree)) return true;

	if (matchWhile(astree)) return true;

	if (matchFor(astree)) return true;

	if (matchDef(astree)) return true;

	if (matchFunc(astree)) return true;

	if (matchClosure(astree)) return true;

	if (matchExpr(astree)) return true;


	if (matchBreak(astree)) return true;
	if (matchContinue(astree)) return true;
	if (matchReturn(astree)) return true;

	return false;
}

SABER_NAMESPACE_END