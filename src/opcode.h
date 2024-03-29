#ifndef H_OPCODE_H
#define H_OPCODE_H

#include "common.h"

SABER_NAMESPACE_BEGIN

class Opcode{
public:
	enum{
		POP      = 0,  //0个参数，将值压出栈顶
		NEG      = 1,  //0个参数，将栈顶值取负
		ADD      = 2,  //0个参数，操作栈sp-1, sp-2
		SUB      = 3,  //0个参数，操作栈sp-1, sp-2
		MUL      = 4,  //0个参数，操作栈sp-1, sp-2
		DIV      = 5,  //0个参数，操作栈sp-1, sp-2
		MOD      = 6,  //0个参数，操作栈sp-1, sp-2
		PLUSEQ   = 7,  //0个参数，操作栈sp-1, sp-2
		MIMUSEQ  = 8,  //0个参数，操作栈sp-1, sp-2
		MULEQ    = 9,  //0个参数，操作栈sp-1, sp-2
		DIVEQ    = 10, //0个参数，操作栈sp-1, sp-2
		MODEQ    = 11, //0个参数，操作栈sp-1, sp-2
		GT       = 12, //0个参数，操作栈sp-1, sp-2
		LT       = 13, //0个参数，操作栈sp-1, sp-2
		GE       = 14, //0个参数，操作栈sp-1, sp-2
		LE       = 15, //0个参数，操作栈sp-1, sp-2
		EQ       = 16, //0个参数，操作栈sp-1, sp-2
		NE       = 17, //0个参数，操作栈sp-1, sp-2
		OR       = 18, //0个参数，操作栈sp-1, sp-2
		AND      = 19, //0个参数，操作栈sp-1, sp-2
		NOT      = 20, 
		SETTABLE = 21, //0个参数
		STFILED  = 22, //0个参数, 操作栈sp-1, sp-2, sp-3
		EXIT     = 23, //0个参数，退出程序
		PUSHN    = 24, //0个参数，将null压入栈顶
		GETLEN   = 25, //0个参数, #指令
		NOP      = 26, //0个参数，占位指令
		ENUM0    = 27, //0个参数指令数量
		GTFILED  = 27, //1个参数，操作栈sp-1, sp-2
		MOVE     = 28, //1个参数，将栈顶的值赋值给参数
		JZ       = 29, //1个参数，比较栈顶的值是否为0, 跳转到参数所指定的地址
		JNZ      = 30, 
		JUMP     = 31, //1个参数，跳转到参数所指定的地址
		CALL     = 32, //1个参数，栈顶值为函数地址，参数为函数参数个数
		TAILCALL = 33, //1个参数
		RET      = 34, //1个参数，返回地址在栈顶
		RESERVE  = 35, //1个参数，栈中保留n个空位供局部变量使用
		PUSHB    = 36, //1个参数，将布尔值压入栈顶
		PUSHI    = 37, //1个参数，将整数压入栈顶
		PUSHF    = 38, //1个参数，将浮点数压入栈顶
		PUSHS    = 39, //1个参数，将字符串压入栈顶
		ENUM1    = 40, //1个参数指令数量
		PUSH     = 40, //2个参数，将值压入栈顶
		ENUM2    = 41, //2个参数指令数量
		ECNUM    = 41,
	};
};

SABER_NAMESPACE_END

#endif