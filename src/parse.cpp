#include "parse.h"
#include "error.h"

SABER_NAMESPACE_BEGIN

void SyntaxParse::Parse(Lexer& lex){
	astProgram = shared_ptr<Astree>(new AstProgram());
	lexer = lex;

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
		if (matchRValue(astReturn) == 2){
			dynamic_cast<AstReturn*>(astReturn.get())->SetMaybeTailCall(true);
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

int SyntaxParse::matchRValue(shared_ptr<Astree>& astree){
	shared_ptr<Astree> stat = shared_ptr<Astree>(new AstStatement());
	if (matchClosure(stat)){
		astree->AddChild(stat);
		return 1;
	}
	else if (matchOrExpr(stat)){
		astree->AddChild(stat);
		return 2;
	}
	else if (matchTable(stat)){
		astree->AddChild(stat);
		return 4;
	}

	return 0;
}

//term       : identifier termTail
//termTail   : . identifier termTail | [ identifier ] termTail
bool SyntaxParse::matchLValueTail(shared_ptr<Astree>& astree, shared_ptr<Astree>& term){
	bool fromFunc = !term.get();
	Token* tok;
	if (match(".", &tok)){
		shared_ptr<Astree> dot = shared_ptr<Astree>(new AstDot());
		dot->SetToken(tok);
		shared_ptr<Astree> name = shared_ptr<Astree>(new AstPrimary());
		if (!matchIdentifier(name)) return false;
		name->SetTable(true);
		if(!fromFunc) dot->AddChild(term);
		dot->AddChild(name);

		return matchLValueTail(astree, dot);
	}
	else if (match("[", &tok)){
		shared_ptr<Astree> dot = shared_ptr<Astree>(new AstDot());
		dot->SetToken(tok);
		shared_ptr<Astree> name = shared_ptr<Astree>(new AstStatement());
		if (!matchExpr(name)) return false;

		if (!match("]", &tok)){
			Error::GetInstance()->ProcessError("����:%d, table�﷨����ȱ��']'", tok->GetLineNumber());
			return false;
		}
		if (!fromFunc) dot->AddChild(term);
		dot->AddChild(name);

		return matchLValueTail(astree, dot);
	}
	else if (!fromFunc){
		astree->AddChild(term);
		return true;
	}
	else{
		return false;
	}
}

bool SyntaxParse::matchLValue(shared_ptr<Astree>& astree){
	shared_ptr<Astree> name = shared_ptr<Astree>(new AstPrimary());
	if (!matchIdentifier(name)) return false;

	return matchLValueTail(astree, name);
}

bool SyntaxParse::matchTerm(shared_ptr<Astree>& astree){
	if (matchFunc(astree)) return true;
	shared_ptr<Astree> left = shared_ptr<Astree>(new AstPrimary());
	if (matchPrimary(left)) {
		astree = left;
		return true;
	}

	shared_ptr<Astree> stat = shared_ptr<Astree>(new AstStatement());
	if (matchLValue(stat)){
		astree = stat;

		return true;
	}

	Token* tok;
	if (match("(", &tok)){
		if (!matchOrExpr(astree)) return false;
		if (!match(")")){
			Error::GetInstance()->ProcessError("����:%d, ���ʽ�﷨����,ȱ��')'", tok->GetLineNumber());
			return false;
		}

		return true;
	}

	if (match("#", &tok)){
		shared_ptr<Astree> astHash = shared_ptr<Astree>(new AstHash());
		astree = astHash;
		shared_ptr<Astree> stat = shared_ptr<Astree>(new AstStatement());
		if (matchClosure(stat)){
			astree->AddChild(stat);
			return true;
		}
		else if (matchTerm(stat)){
			astree->AddChild(stat);
			return true;
		}
		else if (matchTable(stat)){
			astree->AddChild(stat);
			return true;
		}

		Error::GetInstance()->ProcessError("����:%d,[#]ֻ��Ӧ������ֵ��", tok->GetLineNumber());
	}

	return false;
}

bool SyntaxParse::matchNegnotExpr(shared_ptr<Astree>& astree){
	Token* tok;
	if (match("-", &tok) || match("!", &tok)){
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

bool SyntaxParse::matchMuldivExprTail(shared_ptr<Astree>& astree, shared_ptr<Astree>& term){
	Token* tok;
	shared_ptr<Astree> op = shared_ptr<Astree>(new AstOperator());
	if (match("*", &tok) || match("/", &tok) ||
		match("*=", &tok) || match("/=", &tok) ||
		match("%", &tok) || match("%=", &tok)){
		if (tok->GetTokenType() == ETokenType::EOPERATOR){
			if (tok->GetToken() == "*=" || tok->GetToken() == "/=" || tok->GetToken() == "%="){
				shared_ptr<Astree> t = term;
				while (t->GetNumChildren())	t = t->GetChild(0);
				
				if (!t->GetToken() || t->GetToken()->GetTokenType() != ETokenType::EIDENTIFIER){
					Error::GetInstance()->ProcessError("����:%d, %s : �����������Ϊ��ֵ", tok->GetLineNumber(), tok->GetToken().c_str());
					return false;
				}
			}
			shared_ptr<Astree> stat = shared_ptr<Astree>(new AstStatement());
			op->SetToken(tok);
			op->AddChild(term);
			if (!matchNegnotExpr(stat)) return false;
			op->AddChild(stat);
			return matchMuldivExprTail(astree, op);
		}

		lexer.Back();
	}
	else{
		astree->AddChild(term);
		return true;
	}
}

bool SyntaxParse::matchMuldivExpr(shared_ptr<Astree>& astree){
	shared_ptr<Astree> stat = shared_ptr<Astree>(new AstStatement());
	if (!matchNegnotExpr(stat)) return false;

	return matchMuldivExprTail(astree, stat);
}

bool SyntaxParse::matchAddsubExprTail(shared_ptr<Astree>& astree, shared_ptr<Astree>& term){
	Token* tok;
	shared_ptr<Astree> op = shared_ptr<Astree>(new AstOperator());
	if (match("+", &tok) || match("-", &tok) || match("+=", &tok) || match("-=", &tok)){
		if (tok->GetTokenType() == ETokenType::EOPERATOR){
			if (tok->GetToken() == "+=" || tok->GetToken() == "-="){
				shared_ptr<Astree> t = term;
				while (t->GetNumChildren())	t = t->GetChild(0);

				if (!t->GetToken() || t->GetToken()->GetTokenType() != ETokenType::EIDENTIFIER){
					Error::GetInstance()->ProcessError("����:%d, %s : �����������Ϊ��ֵ", tok->GetLineNumber(), tok->GetToken().c_str());
					return false;
				}
			}
			op->SetToken(tok);
			op->AddChild(term);
			if (!matchMuldivExpr(op)) return false;
			return matchAddsubExprTail(astree, op);
		}

		lexer.Back();
	}
	else{
		astree->AddChild(term);
		return true;
	}
}

bool SyntaxParse::matchAddsubExpr(shared_ptr<Astree>& astree){
	shared_ptr<Astree> stat = shared_ptr<Astree>(new AstStatement());
	if (!matchMuldivExpr(stat)) return false;
	
	return matchAddsubExprTail(astree, stat);
}

bool SyntaxParse::matchCompExprTail(shared_ptr<Astree>& astree, shared_ptr<Astree>& term){
	Token* tok;
	shared_ptr<Astree> op = shared_ptr<Astree>(new AstOperator());
	if (match("==", &tok) || match("!=", &tok) ||
		match(">", &tok) || match("<", &tok) ||
		match(">=", &tok) || match("<=", &tok)){
		if (tok->GetTokenType() == ETokenType::EOPERATOR){
			op->SetToken(tok);
			op->AddChild(term);
			if (!matchAddsubExpr(op)) return false;
			return matchCompExprTail(astree, op);
		}

		lexer.Back();
	}
	else{
		astree->AddChild(term);
		return true;
	}
}

bool SyntaxParse::matchCompExpr(shared_ptr<Astree>& astree){
	shared_ptr<Astree> stat = shared_ptr<Astree>(new AstStatement());
	if (!matchAddsubExpr(stat)) return false;

	return matchCompExprTail(astree, stat);
}

bool SyntaxParse::matchAndExprTail(shared_ptr<Astree>& astree, shared_ptr<Astree>& term){
	Token* tok;
	shared_ptr<Astree> op = shared_ptr<Astree>(new AstOperator());
	if (match("&&", &tok)){
		if (tok->GetTokenType() == ETokenType::EOPERATOR){
			op->SetToken(tok);
			op->AddChild(term);
			if (!matchCompExpr(op)) return false;
			return matchAndExprTail(astree, op);
		}

		lexer.Back();
	}
	else{
		astree->AddChild(term);
		return true;
	}
}

bool SyntaxParse::matchAndExpr(shared_ptr<Astree>& astree){
	shared_ptr<Astree> stat = shared_ptr<Astree>(new AstStatement());
	if (!matchCompExpr(stat)) return false;

	return matchAndExprTail(astree, stat);
}

//����������ȼ��Ȼ��
bool SyntaxParse::matchOrExprTail(shared_ptr<Astree>& astree, shared_ptr<Astree>& term){
	Token* tok;
	shared_ptr<Astree> op = shared_ptr<Astree>(new AstOperator());
	if (match("||", &tok)){
		if (tok->GetTokenType() == ETokenType::EOPERATOR){
			op->SetToken(tok);
			op->AddChild(term);
			if (!matchAndExpr(op)) return false;
			return matchOrExprTail(astree, op);
		}

		lexer.Back();
	}
	else{
		astree->AddChild(term);
		return true;
	}
}
bool SyntaxParse::matchOrExpr(shared_ptr<Astree>& astree){
	shared_ptr<Astree> stat = shared_ptr<Astree>(new AstStatement());
	if (!matchAndExpr(stat)) return false;

	return matchOrExprTail(astree, stat);
}

bool SyntaxParse::matchAssignExpr(shared_ptr<Astree>& astree, bool norFor){
	Token* tok = nullptr, *local;
	if (match("local", &local)){
		vector<shared_ptr<Astree>> locals;
		do{
			shared_ptr<Astree> l = shared_ptr<Astree>(new AstLocal());
			l->SetToken(local);
			int childs = locals.size();
			shared_ptr<Astree> name = shared_ptr<Astree>(new AstStatement());
			if (matchLValue(name)){
				l->AddChild(name);
			}
			else if (childs != 0){
				Error::GetInstance()->ProcessError("����:%d, ,��ȱ�ٱ��ʽ", tok->GetLineNumber());
				return false;
			}

			locals.push_back(l);
		} while (match(",", &tok));

		shared_ptr<Astree> op = shared_ptr<Astree>(new AstOperator());
		if (match("=", &tok)){
			op->SetToken(tok);
			int size = locals.size();
			for (int i = 0; i < size; ++i){
				op->AddChild(locals[i]);
			}
			do{
				if (!matchRValue(op)){
					Error::GetInstance()->ProcessError("����:%d, ��ֵ����ȱ�ٱ��ʽ", tok->GetLineNumber());
					return false;
				}
			} while (!norFor && match(",", &tok));

			if (op->GetNumChildren() == size) return false;
			(dynamic_cast<AstOperator*>(op.get())->SetNumVariables(size));
			astree->AddChild(op);

			return true;
		}

		for (int i = 0; i < locals.size();++i)
			astree->AddChild(locals[i]);
		
		return true;
	}

	int p = lexer.GetTkptr(); 
	vector<shared_ptr<Astree>> globals;
	vector<bool> vts;
	do{
		shared_ptr<Astree> g = shared_ptr<Astree>(new AstGlobal());
		int childs = g->GetNumChildren();
		shared_ptr<Astree> name = shared_ptr<Astree>(new AstStatement());
		if (matchLValue(name)){
			bool istable = !dynamic_cast<AstPrimary*>(name->GetChild(0).get());
			g->SetTable(istable);
			g->AddChild(name);

			globals.push_back(g);
			vts.push_back(istable);
		}
		else if (childs != 0){
			Error::GetInstance()->ProcessError("����:%d, ,��ȱ�ٱ��ʽ", tok->GetLineNumber());
			return false;
		}
	} while (match(",", &tok));
	
	if (match("=", &tok)){
		shared_ptr<Astree> op = shared_ptr<Astree>(new AstOperator());
		op->SetToken(tok);
		int size = globals.size();
		for (int i = 0; i < size; ++i){
			op->AddChild(globals[i]);
			op->SetTable(vts[i], i);
		}
		do{
			if (!matchRValue(op)){
				Error::GetInstance()->ProcessError("����:%d, ��ֵ����ȱ�ٱ��ʽ", tok->GetLineNumber());
				return false;
			}
		} while (!norFor && match(",", &tok));

		if (op->GetNumChildren() == size) return false;
		(dynamic_cast<AstOperator*>(op.get())->SetNumVariables(size));
		astree->AddChild(op);

		return true;
	}

	lexer.SetTkptr(p);

	return matchOrExpr(astree);
}

bool SyntaxParse::matchExpr(shared_ptr<Astree>& astree, bool norFor){
	return matchAssignExpr(astree, norFor);
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
		Error::GetInstance()->ProcessError("����:%d, if����﷨����,ȱ�ٹؼ���[then]", iftok->GetLineNumber());
		return false;
	}
	shared_ptr<Astree> stat = shared_ptr<Astree>(new AstStatement());
	astIf->AddChild(stat);
	while (true){
		shared_ptr<Astree> statement = shared_ptr<Astree>(new AstStatement());
		stat->AddChild(statement);
		if (!matchStatement(statement)) break;
	}

	Token* eliftok;
	while (match("elif", &eliftok)){
		shared_ptr<Astree> elif = shared_ptr<Astree>(new AstElif());
		astIf->AddChild(elif);
		shared_ptr<Astree> expr = shared_ptr<Astree>(new AstStatement());
		if (!matchExpr(expr)) return false;
		elif->AddChild(expr);
		if (!match("then")){
			Error::GetInstance()->ProcessError("����:%d, elif����﷨����,ȱ�ٹؼ���[then]", eliftok->GetLineNumber());
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
		Error::GetInstance()->ProcessError("����:%d, while����﷨����,ȱ�ٹؼ���[do]", tok->GetLineNumber());

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

bool SyntaxParse::matchForNormal(shared_ptr<Astree>& astree){
	shared_ptr<Astree> astFor = shared_ptr<Astree>(new AstFor());
	Token* tok;
	if (!match("for", &tok)) return false;
	astFor->SetToken(tok);
	astree->AddChild(astFor);
	shared_ptr<Astree> expr = shared_ptr<Astree>(new AstStatement());
	if (!matchExpr(expr, true)) return false;
	astFor->AddChild(expr);
	if (!match(",")){
		Error::GetInstance()->ProcessError("����:%d, for����﷨����,ѭ��������ȱ��[,]", tok->GetLineNumber());
		return false;
	}
	shared_ptr<Astree> cond = shared_ptr<Astree>(new AstStatement());
	if (!matchExpr(cond, true)) return false;
	astFor->AddChild(cond);
	shared_ptr<Astree> step = shared_ptr<Astree>(new AstStatement());
	if (match(",")){
		//normal
		if(matchExpr(step, true)) astFor->AddChild(step);
		else{
			Error::GetInstance()->ProcessError("����:%d, for����﷨����,ȱ��step����", tok->GetLineNumber());
			return false;
		}
	}

	if (!match("do")){
		Error::GetInstance()->ProcessError("����:%d, for����﷨����,ȱ�ٹؼ���[do]", tok->GetLineNumber());
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

bool SyntaxParse::matchForGeneric(shared_ptr<Astree>& astree){
	shared_ptr<Astree> astFor = shared_ptr<Astree>(new AstFor());
	int p = lexer.GetTkptr();
	Token* tok;
	if (!match("for", &tok)) return false;
	astFor->SetToken(tok);
	if (match("local")){
		shared_ptr<Astree> l = shared_ptr<Astree>(new AstLocal());
		shared_ptr<Astree> name = shared_ptr<Astree>(new AstPrimary());
		if (!matchIdentifier(name)){
			lexer.SetTkptr(p);
			return false;
		}

		l->AddChild(name);
		astFor->AddChild(l);
	}
	else{
		shared_ptr<Astree> g = shared_ptr<Astree>(new AstGlobal());
		shared_ptr<Astree> name = shared_ptr<Astree>(new AstPrimary());
		if (!matchIdentifier(name)){
			lexer.SetTkptr(p);
			return false;
		}

		g->AddChild(name);
		astFor->AddChild(g);
	}

	if (!match("in")){
		lexer.SetTkptr(p);
		return false;
	}

	(dynamic_cast<AstFor*>(astFor.get()))->SetGeneric();
	astree->AddChild(astFor);

	shared_ptr<Astree> func = shared_ptr<Astree>(new AstStatement());
	if (!matchFunc(func)){
		Error::GetInstance()->ProcessError("����:%d, for����﷨����,ȱ�ٵ�����", tok->GetLineNumber());
		return false;
	}
	astFor->AddChild(func);

	if (!match("do")){
		Error::GetInstance()->ProcessError("����:%d, for����﷨����,ȱ�ٹؼ���[do]", tok->GetLineNumber());
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
			Error::GetInstance()->ProcessError("����:%d, ���������﷨����,��������������ĸ��ͷ������ĸ���������", tok->GetLineNumber());
			return false;
		}

		lexer.Back();

		return false;
	}
	if (!match("(")){
		Error::GetInstance()->ProcessError("����:%d, ���������﷨����,ȱ��'('", tok->GetLineNumber());
		return false;
	}

	do{
		Token* tdot;
		int childs = def->GetNumChildren();
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
		else if(childs != 1){
			Error::GetInstance()->ProcessError("����:%d, ���������﷨����,','����ȱ�ٲ���", tok->GetLineNumber());
			return false;
		}
	} while (match(",", &tok));

	if (!match(")")){
		Error::GetInstance()->ProcessError("����:%d, ���������﷨����,ȱ��')'", tok->GetLineNumber());
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
	shared_ptr<Astree> name = shared_ptr<Astree>(new AstStatement());
	Token* tok = nullptr;
	int p = lexer.GetTkptr();
	if (!fromClosure){
		if (!matchLValue(name)) return false;

		func->AddChild(name);
	}

	if (!match("(", &tok)){
		lexer.SetTkptr(p);
		return false;
	}

	(dynamic_cast<AstFunc*>(func.get())->SetFunc(fromClosure));

	do{	
		shared_ptr<Astree> parent = shared_ptr<Astree>(new AstStatement());
		do{
			int childs = parent->GetNumChildren();
			shared_ptr<Astree> param = shared_ptr<Astree>(new AstStatement());
			if (matchRValue(param)){
				parent->AddChild(param);
			}
			else if (childs != 0){
				Error::GetInstance()->ProcessError("����:%d, ���������﷨����,','����ȱ�ٲ���", tok->GetLineNumber());
				return false;
			}
		} while (match(",", &tok));
		func->AddChild(parent);

		if (!match(")")){
			Error::GetInstance()->ProcessError("����:%d, ���������﷨����,ȱ��')'", tok->GetLineNumber());
			return false;
		}
	} while (match("(", &tok));

	astree->AddChild(func);

	name = shared_ptr<Astree>(new AstStatement());
	shared_ptr<Astree> dumy;
	if (matchLValueTail(name, dumy)){
		astree->AddChild(name);
		matchFunc(astree, true);
	}

	return true;
}

bool SyntaxParse::matchClosure(shared_ptr<Astree>& astree){
	shared_ptr<Astree> closure = shared_ptr<Astree>(new AstClosure());
	int p = lexer.GetTkptr();
	int leftParentheses = 0;
	while (match("(")) leftParentheses++;
	Token* tok;
	if (!match("def", &tok)){
		lexer.SetTkptr(p);
		return false;
	}
	if (!match("(")){
		Error::GetInstance()->ProcessError("����:%d, ���������﷨����,ȱ��'('", tok->GetLineNumber());
		return false;
	}

	do{
		Token* tdot;
		int childs = closure->GetNumChildren();
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
		else if (childs != 0){
			Error::GetInstance()->ProcessError("����:%d, ���������﷨����,','����ȱ�ٲ���", tok->GetLineNumber());
			return false;
		}
	} while (match(",", &tok));

	if (!match(")")){
		Error::GetInstance()->ProcessError("����:%d, ���������﷨����,ȱ��')'", tok->GetLineNumber());
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

	int rightParentheses = leftParentheses;
	while (rightParentheses--) if (!match(")")) break;
	
	if (leftParentheses != 0){
		if (rightParentheses != -1){
			Error::GetInstance()->ProcessError("����:%d, �������������﷨����,ȱ��')'", tok->GetLineNumber());
			return false;
		}

		matchFunc(astree, true);
	}
	else if (match("(")){
		Error::GetInstance()->ProcessError("����:%d, �������������﷨����", tok->GetLineNumber());
		return false;
	}

	return true;
}

bool SyntaxParse::matchTableInit(shared_ptr<Astree>& astree){
	shared_ptr<Astree> name = shared_ptr<Astree>(new AstPrimary());
	if (!matchString(name) && !matchIdentifier(name)) return false;
	name->SetTable(true);
	Token* tok;
	if (match("=", &tok)){
		shared_ptr<Astree> op = shared_ptr<Astree>(new AstOperator());
		op->SetTable(true);
		dynamic_cast<AstOperator*>(op.get())->SetTableInit();
		op->SetToken(tok);
		op->AddChild(name);
		if (!matchRValue(op)) return false;

		astree->AddChild(op);
	}

	return true;
}

bool SyntaxParse::matchTable(shared_ptr<Astree>& astree){
	Token* tok;
	if (match("{", &tok)){
		shared_ptr<Astree> t = shared_ptr<Astree>(new AstTable());
		astree->AddChild(t);
		do{
			int childs = t->GetNumChildren();
			shared_ptr<Astree> ast = shared_ptr<Astree>(new AstStatement());
			if (matchTableInit(ast)){
				t->AddChild(ast);
			}
			else if (childs != 0){
				Error::GetInstance()->ProcessError("����:%d, ���������﷨����,','����ȱ�ٲ���", tok->GetLineNumber());
				return false;
			}
		} while (match(",", &tok));

		if (!match("}")){
			Error::GetInstance()->ProcessError("����:%d, ���﷨����ȱ��}", tok->GetLineNumber());
			return false;
		}

		return true;
	}

	return false;
}

bool SyntaxParse::matchStatement(shared_ptr<Astree>& astree){
	if (matchIf(astree)) return true;

	if (matchWhile(astree)) return true;

	if (matchForGeneric(astree)) return true;

	if (matchForNormal(astree)) return true;

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