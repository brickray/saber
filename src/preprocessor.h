#ifndef H_PREPROCESSOR_H
#define H_PREPROCESSOR_H

#include "common.h"

SABER_NAMESPACE_BEGIN

class Preprocessor{
public:
	string Process(string& code);

private:
	string comments(string& code);
};

SABER_NAMESPACE_END

#endif