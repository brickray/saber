#ifndef H_OPCODE_H
#define H_OPCODE_H

#include "common.h"

class Opcode{
public:
	enum{
		MOVE    = 0,
		JZ      = 1,
		JUMP    = 2,
		CALL    = 3,
		RET     = 4,
		PUSH    = 5,
		POP     = 6,
		SAVE    = 7,
		RESTORE = 8,
		NEG     = 9,
		ADD     = 10,
		SUB     = 11,
		MUL     = 12,
		DIV     = 13,
		MOD     = 14,
		GT      = 15, //greater than
		LT      = 16, //less than
		GE      = 17, //greater equal than
		LE      = 18, //less equal than
		EQ      = 19,
		NE      = 20, //not equal
		OR      = 21,
		AND     = 22,
		NOP     = 23, //do nothing
		ECNUM   = 24,
	};
};

#endif