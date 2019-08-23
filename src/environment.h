#ifndef H_ENVIRONMENT_H
#define H_ENVIRONMENT_H

#include "common.h"
#include "value.h"

SABER_NAMESPACE_BEGIN

struct SymbolInfo{
	Value value;
	int address;
	bool local;
};

class Environment{
protected:
	shared_ptr<Environment> outter;
	hash_map<string, SymbolInfo> symbolTable;
	hash_map<string, SymbolInfo> constantTable;

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
		if (hasSymbol(symbol)) return getSymbol(symbol);

		if (outter.get()) return outter->GetSymbol(symbol);

		return SymbolInfo();
	}

	SymbolInfo GetSymbol(string symbol, bool& local){
		if (hasSymbol(symbol)){
			SymbolInfo si = getSymbol(symbol);
			local = si.local;
			return si;
		}

		if (outter.get()) return outter->GetSymbol(symbol, local);

		return SymbolInfo();
	}

	bool HasSymbol(string symbol) const{
		if (hasSymbol(symbol))
			return true;

		if (outter.get() && outter->HasSymbol(symbol)) 
			return true;

		return false;
	}

private:
	SymbolInfo getSymbol(string symbol){
		return symbolTable[symbol];
	}

	bool hasSymbol(string symbol) const{
		return symbolTable.find(symbol) != symbolTable.end();
	}
};

SABER_NAMESPACE_END

#endif