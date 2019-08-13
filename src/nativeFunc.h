#ifndef H_NATIVEFUNC_H
#define H_NATIVEFUNC_H

#include "common.h"
#include "environment.h"

struct RegisterFunction{
	string name;
	SFunc   f;
};

class NativeFunc{
public:
	static void Register(shared_ptr<Environment>& e);
};

#endif