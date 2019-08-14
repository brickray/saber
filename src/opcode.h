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
		CALLN   = 4,
		RET     = 5,
		PUSH    = 6,
		POP     = 7,
		SAVE    = 8,
		RESTORE = 9,
		NEG     = 10,
		ADD     = 11,
		SUB     = 12,
		MUL     = 13,
		DIV     = 14,
		MOD     = 15,
		GT      = 16, //greater than
		LT      = 17, //less than
		GE      = 18, //greater equal than
		LE      = 19, //less equal than
		EQ      = 20,
		NE      = 21, //not equal
		NOP     = 22, //do nothing
		ECNUM   = 23,
	};
};

#endif