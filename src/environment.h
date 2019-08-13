#ifndef H_ENVIRONMENT_H
#define H_ENVIRONMENT_H

#include "common.h"
#include "value.h"

struct SymbolInfo{
	Value value;
	int address;
};

class Environment{
protected:
	shared_ptr<Environment> outter;
	hash_map<string, SymbolInfo> symbolTable;

public:
	Environment() : outter(nullptr){

	}

	void SetOutter(shared_ptr<Environment>& out){
		outter = out;
	}

	shared_ptr<Environment> GetOutter() const{
		return outter;
	}

	void SetSymbol(string symbol, SymbolInfo si){
		symbolTable[symbol] = si;
	}

	SymbolInfo GetSymbol(string symbol){
		if (HasSymbol(symbol)) return symbolTable[symbol];

		return SymbolInfo();
	}

	bool HasSymbol(string symbol){
		return symbolTable.find(symbol) != symbolTable.end();
	}
};

#endif