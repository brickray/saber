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
	hash_map<string, SymbolInfo> stringTable;
	hash_map<string, SymbolInfo> numberTable;

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

		if (outter) return outter->GetSymbol(symbol);

		return SymbolInfo();
	}

	SymbolInfo GetSymbol(string symbol, bool& local){
		if (hasSymbol(symbol)){
			SymbolInfo si = getSymbol(symbol);
			local = si.local;
			return si;
		}

		if (outter) return outter->GetSymbol(symbol, local);

		return SymbolInfo();
	}

	bool HasSymbol(string symbol) const{
		if (hasSymbol(symbol))
			return true;

		if (outter.get() && outter->HasSymbol(symbol)) 
			return true;

		return false;
	}

	void SetStringSymbol(string symbol, SymbolInfo si){
		if (outter) outter->SetStringSymbol(symbol, si);

		stringTable[symbol] = si;
	}

	SymbolInfo GetStringSymbol(string symbol){
		if (outter) outter->GetStringSymbol(symbol);

		if (hasStringSymbol(symbol))
			return getStringSymbol(symbol);

		return SymbolInfo();
	}

	bool HasStringSymbol(string symbol){
		if (outter) outter->HasStringSymbol(symbol);

		return hasStringSymbol(symbol);
	}

	void SetNumberSymbol(string symbol, SymbolInfo si){
		if (outter) outter->SetNumberSymbol(symbol, si);

		numberTable[symbol] = si;
	}

	SymbolInfo GetNumberSymbol(string symbol){
		if (outter) outter->GetNumberSymbol(symbol);

		if (hasNumberSymbol(symbol))
			return getNumberSymbol(symbol);

		return SymbolInfo();
	}

	bool HasNumberSymbol(string symbol){
		if (outter) outter->HasNumberSymbol(symbol);

		return hasNumberSymbol(symbol);
	}

private:
	SymbolInfo getSymbol(string symbol){
		return symbolTable[symbol];
	}

	bool hasSymbol(string symbol) const{
		return symbolTable.find(symbol) != symbolTable.end();
	}

	SymbolInfo getStringSymbol(string symbol){
		return stringTable[symbol];
	}

	bool hasStringSymbol(string symbol) const{
		return stringTable.find(symbol) != stringTable.end();
	}

	SymbolInfo getNumberSymbol(string symbol){
		return numberTable[symbol];
	}

	bool hasNumberSymbol(string symbol) const{
		return numberTable.find(symbol) != numberTable.end();
	}
};

SABER_NAMESPACE_END

#endif