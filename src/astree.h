#ifndef H_ASTREE_H
#define H_ASTREE_H

#include "common.h"
#include "lexer.h"
#include "value.h"
#include "environment.h"
#include "vm.h"
#include "opcode.h"

class Astree{
protected:
	Token* token = nullptr;
	vector<shared_ptr<Astree>> children;

public:
	void SetToken(Token* tok) { token = tok; }
	Token* GetToken() { return token; }
	int GetNumChildren() const { return children.size(); }
	shared_ptr<Astree > GetChild(int n) { return children[n]; }
	void AddChild(shared_ptr<Astree> ast){
		children.push_back(ast);
	}

	virtual int Eval(shared_ptr<Environment>& e, shared_ptr<VM>& vm) = 0;
	virtual void Compile(){};

	virtual string ToString() const{
		string ret;
		if (token){
			ret += token->GetToken();
			ret += " ";
		}
		for (int i = 0; i < children.size(); ++i){
			ret += children[i]->ToString();
			ret += " ";
		}

		return ret;
	}
};

#endif