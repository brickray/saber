#include "error.h"
#include <stdarg.h>


SABER_NAMESPACE_BEGIN

Error* Error::instance = nullptr;

void Error::ProcessError(const char* format, ...){
	va_list args;
	va_start(args, format);
	char errorBuf[2048];
	vsnprintf_s(errorBuf, sizeof(errorBuf), _TRUNCATE, format, args);

	printf("%s\n", errorBuf);

	va_end(args);

	system("pause");
}

SABER_NAMESPACE_END