#ifndef H_OPCODE_H
#define H_OPCODE_H

#include "common.h"

class Opcode{
public:
	enum{
		LOAD    = 0,
		MOVE    = 1,
		JZ      = 2,
		JUMP    = 3,
		CALL    = 4,
		CALLN   = 5,
		RET     = 6,
		SAVE    = 7,
		RESTORE = 8,
		NEG     = 9,
		ADD     = 10,
		SUB     = 11,
		MUL     = 12,
		DIV     = 13,
		MOD     = 14,
		GT      = 15, //greater than
		GE      = 16, //greater equal than
		EQ      = 17,
		NE      = 18, //not equal
		NOP     = 19, //do nothing
		ECNUM   = 20,
	};
};

#endif