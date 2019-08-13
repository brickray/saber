#ifndef H_AST_ASTFUNC_H
#define H_AST_ASTFUNC_H

#include "../astree.h"
#include "astDef.h"

class AstFunc : public Astree{
public:
	virtual int Eval(shared_ptr<Environment>& e, shared_ptr<VM>& vm){
		Token* tok = children[0]->GetToken();
		string funcName = tok->GetToken();
		if (!e->HasSymbol(funcName)){
			printf("行数:%d, 未定义函数名[%s]\n", tok->GetLineNumber(), funcName.c_str());
			return 0;
		}
		bool isFunc = e->GetSymbol(funcName).value.IsFunction();
		bool isNativeFunc = e->GetSymbol(funcName).value.IsNativeFunction();
		if (!isFunc && !isNativeFunc){
			printf("行数:%d,[%s]不是函数\n", tok->GetLineNumber(), funcName.c_str());
			return 0;
		}
		if (isFunc){
			shared_ptr<Astree> func = e->GetSymbol(funcName).value.GetFunction();
			AstDef* rfunc = dynamic_cast<AstDef*>(func.get());
			int numChilds = rfunc->GetNumChildren();
			int numRetParams = rfunc->GetNumReturnParams();
			if (children.size() - 1 != numChilds - 2 - numRetParams){
				printf("行数:%d, 函数参数不一致\n", tok->GetLineNumber());
				return 0;
			}

			shared_ptr<Environment> local = shared_ptr<Environment>(new Environment());
			shared_ptr<Environment> root = e;
			while (root->GetOutter().get())	root = root->GetOutter();

			local->SetOutter(root);
			//set local variable
			/*for (int i = 0; i < numChilds - 2 - numRetParams; ++i){
				Value ret = children[i + 1]->Eval(e, vm);
				string name = rfunc->GetChild(i + 1)->GetToken()->GetToken();
				local->Add(name, ret);
			}
			Value ret = rfunc->GetChild(numChilds - 1 - numRetParams)->Eval(local, vm);
			if (numRetParams > 0){
				ret = rfunc->GetChild(numChilds - 1)->Eval(local, vm);
			}*/
			return 0;
		}
		else{
			int func = e->GetSymbol(funcName).address;
			int numParams = children.size() - 1;
			vector<int> addresses(numParams);
			for (int i = 0; i < numParams; ++i){
				addresses[i] = children[i + 1]->Eval(e, vm);
			}

			vm->AddCode(Opcode::CALLN);
			vm->AddCode(func);
			vm->AddCode(numParams);
			for (int i = 0; i < numParams; ++i){
				vm->AddCode(addresses[i]);
			}

			return 0;
		}
	}
};

#endif