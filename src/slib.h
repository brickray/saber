#ifndef H_SLIB_H
#define H_SLIB_H

#include "common.h"

inline void InitSLib(string& code){
	string pairs = "def pairs(t)\
						if istable(t) == false then\
							return def() return false end\
						end\
				   		local copy = {}\
						table.copy(copy, t)\
						return def()\
							       return next(copy)\
							   end\
					 end\
					 ";

	code = code.insert(0, pairs);
}

#endif