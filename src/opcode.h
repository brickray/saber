#ifndef H_OPCODE_H
#define H_OPCODE_H

#include "common.h"

SABER_NAMESPACE_BEGIN

class Opcode{
public:
	enum{
		JZ      = 0,  //0个参数，比较栈顶的值是否为0
		POP     = 1,  //0个参数，将值压出栈顶
		NEG     = 2,  //现在没用
		ADD     = 3,  //0个参数，操作栈sp-1, sp-2
		SUB     = 4,  //0个参数，操作栈sp-1, sp-2
		MUL     = 5,  //0个参数，操作栈sp-1, sp-2
		DIV     = 6,  //0个参数，操作栈sp-1, sp-2
		MOD     = 7,  //0个参数，操作栈sp-1, sp-2
		GT      = 8,  //0个参数，操作栈sp-1, sp-2
		LT      = 9,  //0个参数，操作栈sp-1, sp-2
		GE      = 10, //0个参数，操作栈sp-1, sp-2
		LE      = 11, //0个参数，操作栈sp-1, sp-2
		EQ      = 12, //0个参数，操作栈sp-1, sp-2
		NE      = 13, //0个参数，操作栈sp-1, sp-2
		OR      = 14, //0个参数，操作栈sp-1, sp-2
		AND     = 15, //0个参数，操作栈sp-1, sp-2
		NOP     = 16, //0各参数，占位指令
		ENUM0   = 17, //0个参数指令数量
		MOVE    = 17, //1个参数，将栈顶的值赋值给参数
		JUMP    = 18, //1个参数，跳转到参数所指定的地址
		CALL    = 19, //1个参数，栈顶值为函数地址，参数为函数参数个数
		RET     = 20, //1个参数，返回地址在栈顶
		PUSH    = 21, //1各参数，将值压入栈顶
		ENUM1   = 22, //1个参数指令数量
		ECNUM   = 22,
	};
};

SABER_NAMESPACE_END

#endif