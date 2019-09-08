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
		GT       = 7,  //0个参数，操作栈sp-1, sp-2
		LT       = 8,  //0个参数，操作栈sp-1, sp-2
		GE       = 9,  //0个参数，操作栈sp-1, sp-2
		LE       = 10, //0个参数，操作栈sp-1, sp-2
		EQ       = 11, //0个参数，操作栈sp-1, sp-2
		NE       = 12, //0个参数，操作栈sp-1, sp-2
		OR       = 13, //0个参数，操作栈sp-1, sp-2
		AND      = 14, //0个参数，操作栈sp-1, sp-2
		SETTABLE = 15, //0个参数
		STFILED  = 16, //0个参数, 操作栈sp-1, sp-2, sp-3
		EXIT     = 17, //0个参数，退出程序
		PUSHN    = 18, //0个参数，将null压入栈顶
		GETLEN   = 19, //0个参数, #指令
		NOP      = 20, //0个参数，占位指令
		ENUM0    = 21, //0个参数指令数量
		GTFILED  = 21, //1个参数，操作栈sp-1, sp-2
		MOVE     = 22, //1个参数，将栈顶的值赋值给参数
		JZ       = 23, //1个参数，比较栈顶的值是否为0, 跳转到参数所指定的地址
		JUMP     = 24, //1个参数，跳转到参数所指定的地址
		CALL     = 25, //1个参数，栈顶值为函数地址，参数为函数参数个数
		RET      = 26, //1个参数，返回地址在栈顶
		RESERVE  = 27, //1个参数，栈中保留n个空位供局部变量使用
		PUSHB    = 28, //1个参数，将布尔值压入栈顶
		PUSHI    = 29, //1个参数，将整数压入栈顶
		PUSHF    = 30, //1个参数，将浮点数压入栈顶
		PUSHS    = 31, //1个参数，将字符串压入栈顶
		ENUM1    = 32, //1个参数指令数量
		PUSH     = 32, //2个参数，将值压入栈顶
		ENUM2    = 33, //2个参数指令数量
		ECNUM    = 33,
	};
};

SABER_NAMESPACE_END

#endif