#ifndef H_ERROR_H
#define H_ERROR_H

#include "common.h"

SABER_NAMESPACE_BEGIN

class Error{
protected:
	static Error* instance;

public:
	static Error* GetInstance(){
		if (instance) return instance;

		instance = new Error();
		return instance;
	}

	static void Destroy(){
		if (instance) delete instance;
	}

	void ProcessError(const char* format, ...);

protected:
	Error(){}
	~Error(){}
};

SABER_NAMESPACE_END

#endif