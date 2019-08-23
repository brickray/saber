#ifndef H_ASTREE_H
#define H_ASTREE_H

#include "common.h"
#include "lexer.h"
#include "value.h"
#include "environment.h"
#include "svm.h"
#include "opcode.h"
#include "error.h"

SABER_NAMESPACE_BEGIN

struct BlockCnt{
	bool isloop;
	vector<int> bps; //break points
	vector<int> cps; //continue points
	vector<int> elifs; //else if
	vector<int> rets; //return points
	int nearst;
	int variableIndex; 

	BlockCnt(){
		isloop = false;
		variableIndex = 0;
	}
};

class Astree{
protected:
	Token* token = nullptr;
	vector<shared_ptr<Astree>> children;

public:
	virtual ~Astree(){};

	void SetToken(Token* tok) { token = tok; }
	Token* GetToken() { return token; }
	int GetNumChildren() const { return children.size(); }
	shared_ptr<Astree > GetChild(int n) { return children[n]; }
	void AddChild(shared_ptr<Astree>& ast){ children.push_back(ast); }

	virtual void Compile(shared_ptr<Environment>& e, shared_ptr<SVM>& svm, BlockCnt& bc) = 0;

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

SABER_NAMESPACE_END

#endif