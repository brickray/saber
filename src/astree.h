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
	bool lasttail;
	bool anyOperator; //只在return语句中使用
	bool needRet;
	int nearst;
	int variableIndex;
	int maxLevel;
	Clptr cl;
	string nearstS;
	vector<int> bps; //break points
	vector<int> cps; //continue points
	vector<int> elifs; //else if

	BlockCnt(){
		isloop = false;
		lasttail = false;
		anyOperator = false;
		needRet = false;
		nearst = 0;
		variableIndex = 0;
		maxLevel = 0;
		cl = nullptr;
	}
};

class Astree{
protected:
	Token* token = nullptr;
	vector<shared_ptr<Astree>> children;
	Integer istable = 0;

public:
	virtual ~Astree(){};

	void SetToken(Token* tok) { token = tok; }
	Token* GetToken() { return token; }
	int GetNumChildren() const { return children.size(); }
	int GetSubtreeNumNodes() const {
		int ret = 0;
		int size = GetNumChildren();
		ret += size;
		for (int i = 0; i < size; ++i){
			ret += GetChild(i)->GetSubtreeNumNodes();
		}

		return ret;
	}
	void GetSubtreeNodes(vector<shared_ptr<Astree>>& c){
		for (int i = 0; i < GetNumChildren(); ++i){
			c.push_back(GetChild(i));
			GetChild(i)->GetSubtreeNodes(c);
		}
	}
	shared_ptr<Astree > GetChild(int n) const { return children[n]; }
	void AddChild(shared_ptr<Astree>& ast){ children.push_back(ast); }
	void RemoveAllChild() { children.clear(); }
	void SetTable(bool t, int idx = 0) { istable |= (t << idx); }

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