#include "parse.h"

void SyntaxParse::Parse(){
	int idx = 0;
	while (!lexer.IsEnd()){
		shared_ptr<Astree> node = shared_ptr<Astree>(new AstStatement());
		if (matchStatement(node)){
			asts.push_back(node);
		}
		else{

		}
	}

	/*for (int i = 0; i < asts.size(); ++i){
		string str = asts[i]->ToString();
		printf("%s\n", str.c_str());
	}*/
}

int SyntaxParse::Eval(shared_ptr<Environment>& e, shared_ptr<VM>& vm){
	int ret;
	for (int i = 0; i < asts.size(); ++i){
		ret = asts[i]->Eval(e, vm);
	}

	return ret;
}

bool SyntaxParse::match(string name){
	Token* tok = lexer.NextToken();
	if (tok->GetToken() == name) return true;

	lexer.Back();
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
	Token* tok = lexer.NextToken();
	if (!tok) return false;
	if (tok->GetTokenType() == ETokenType::ENUMBER ||
		tok->GetTokenType() == ETokenType::EIDENTIFIER ||
		tok->GetTokenType() == ETokenType::ESTRING){
		astree->SetToken(tok);

		return true;
	}

	lexer.Back();
	return false;
}

bool SyntaxParse::matchOp(shared_ptr<Astree>& astree){
	Token* tok = lexer.NextToken();
	if (!tok) return false;

	if (tok->GetTokenType() == ETokenType::EOPERATOR){
		string t = tok->GetToken();
		if (t == "+=" || t == "-=" || t == "*=" || t == "/=" || t == "&="){
			if (lexer.PrevToken()->GetTokenType() != ETokenType::EIDENTIFIER){
				printf("行数:%d, %s:左操作数必须为左值\n", tok->GetLineNumber(), tok->GetToken().c_str());

				return false;
			}
		}

		astree->SetToken(tok);
		return true;
	}

	lexer.Back();
	return false;
}

bool SyntaxParse::matchExpr(shared_ptr<Astree>& astree){
	if (matchFunc(astree)) return true;

	shared_ptr<Astree> left = shared_ptr<Astree>(new AstPrimary());
	if (!matchPrimary(left)) return false;
	shared_ptr<Astree> mid = shared_ptr<Astree>(new AstOperator());
	if (matchOp(mid)){
		mid->AddChild(left);
		shared_ptr<Astree> right = shared_ptr<Astree>(new AstStatement());
		if (!matchExpr(right)) return false;
		mid->AddChild(right);
		astree->AddChild(mid);

		return true;
	}

	astree = left;
	return true;
}

bool SyntaxParse::matchIf(shared_ptr<Astree>& astree){
	shared_ptr<Astree> astIf = shared_ptr<Astree>(new AstIf());
	if (!match("if")) return false;
	shared_ptr<Astree> expr = shared_ptr<Astree>(new AstStatement());
	if (!matchExpr(expr)) return false;
	astIf->AddChild(expr);
	if (!match("then")) return false;
	shared_ptr<Astree> stat = shared_ptr<Astree>(new AstStatement());
	while (true){
		shared_ptr<Astree> statement = shared_ptr<Astree>(new AstStatement());
		if (!matchStatement(statement)) break;
		stat->AddChild(statement);
	}

	astIf->AddChild(stat);

	while (match("elif")){
		shared_ptr<Astree> elif = shared_ptr<Astree>(new AstIf());
		shared_ptr<Astree> expr = shared_ptr<Astree>(new AstStatement());
		if (!matchExpr(expr)) return false;
		elif->AddChild(expr);
		if (!match("then")) return false;
		shared_ptr<Astree> stat = shared_ptr<Astree>(new AstStatement());
		while (true){
			shared_ptr<Astree> statement = shared_ptr<Astree>(new AstStatement());
			if (!matchStatement(statement)) break;
			stat->AddChild(statement);
		}

		elif->AddChild(stat);
		astIf->AddChild(elif);
	}

	if (match("else")){
		dynamic_cast<AstIf*>(astIf.get())->SetElseBlock();
		shared_ptr<Astree> stat = shared_ptr<Astree>(new AstStatement());
		while (true){
			shared_ptr<Astree> statement = shared_ptr<Astree>(new AstStatement());
			if (!matchStatement(statement)) break;
			stat->AddChild(statement);
		}

		astIf->AddChild(stat);
	}

	if (!match("end")) return false;

	astree->AddChild(astIf);

	return true;
}

bool SyntaxParse::matchWhile(shared_ptr<Astree>& astree){
	shared_ptr<Astree> astWhile = shared_ptr<Astree>(new AstWhile());
	if (!match("while")) return false;
	shared_ptr<Astree> expr = shared_ptr<Astree>(new AstStatement());
	if (!matchExpr(expr)) return false;
	astWhile->AddChild(expr);
	if (!match("then")) return false;
	while (true){
		shared_ptr<Astree> statement = shared_ptr<Astree>(new AstStatement());	
		if (!matchStatement(statement)) break;
		astWhile->AddChild(statement);
	}
	if (!match("end")) return false;

	astree->AddChild(astWhile);

	return true;
}

bool SyntaxParse::matchDef(shared_ptr<Astree>& astree){
	shared_ptr<Astree> def = shared_ptr<Astree>(new AstDef());
	if (!match("def")) return false;
	shared_ptr<Astree> fun = shared_ptr<Astree>(new AstPrimary());
	if (matchIdentifier(fun)){
		def->AddChild(fun);
	}
	if (!match("(")) return false;
	while (true){
		shared_ptr<Astree> param = shared_ptr<Astree>(new AstPrimary());
		if (matchIdentifier(param)){
			def->AddChild(param);
		}
		else break;
	}

	if (!match(")")) return false;

	shared_ptr<Astree> stat = shared_ptr<Astree>(new AstStatement());
	while (true){
		shared_ptr<Astree> statement = shared_ptr<Astree>(new AstStatement());
		if (!matchStatement(statement)) break;
		stat->AddChild(statement);
	}

	def->AddChild(stat);

	if (match("return")){
		shared_ptr<Astree> ret = shared_ptr<Astree>(new AstPrimary());
		if (matchPrimary(ret)){
			def->AddChild(ret);
			dynamic_cast<AstDef*>(def.get())->SetNumReturnParams(1);
		}

		match(";");
	}
	
	if (!match("end")) return false;

	astree->AddChild(def);

	return true;
}

bool SyntaxParse::matchFunc(shared_ptr<Astree>& astree){
	shared_ptr<Astree> func = shared_ptr<Astree>(new AstFunc());
	shared_ptr<Astree> name = shared_ptr<Astree>(new AstPrimary());
	if (!matchIdentifier(name)) return false;
	func->AddChild(name);
	
	if (!match("(")){
		lexer.Back();
		return false;
	}
	while (true){
		shared_ptr<Astree> param = shared_ptr<Astree>(new AstStatement());
		if (matchExpr(param)){
			func->AddChild(param);
		}
		else break;
	}

	if (!match(")")) return false;

	match(";");

	astree->AddChild(func);

	return true;
}

bool SyntaxParse::matchStatement(shared_ptr<Astree>& astree){
	shared_ptr<Astree> dummy = shared_ptr<Astree>(new AstPrimary());

	if (matchIf(astree)) return true;

	if (matchWhile(astree)) return true;

	if (matchDef(astree)) return true;

	if (matchFunc(astree)) return true;

	if (matchExpr(astree)){
		match(";");
		return true;
	}

	return false;
}