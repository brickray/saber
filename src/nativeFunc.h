#ifndef H_NATIVEFUNC_H
#define H_NATIVEFUNC_H

#include "common.h"
#include "environment.h"

SABER_NAMESPACE_BEGIN

struct RegisterFunction{
	string name;
	SFunc   f;
};

class NativeFunc{
public:
	static void Register(shared_ptr<Environment>& e, shared_ptr<SVM>& svm);
};

extern void cocallback(SVM* svm);

SABER_NAMESPACE_END

#endif