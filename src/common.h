#ifndef H_COMMON_H
#define H_COMMON_H

#include <string>
#include <vector>
#include <stack>
#include <hash_set>
#include <hash_set>
#include <hash_map>
#include <memory>

using namespace std;

#define SABER_NAMESPACE_BEGIN namespace Saber{
#define SABER_NAMESPACE_END }
#define SABER_NAMESPACE_USING using namespace Saber;

#define SABER_INT_INT         0
#define SABER_INT_LONG        1
#define SABER_INT_LONGLONG    2
#define SABER_INT_TYPE        SABER_INT_LONGLONG
#define SABER_FLOAT_FLOAT     0
#define SABER_FLOAT_DOUBLE    1
#define SABER_FLOAT_TYPE      SABER_FLOAT_FLOAT

#if SABER_INT_TYPE == SABER_INT_INT

typedef int Integer;
#define SABER_MIN_INTEGER INT_MIN
#define SABER_MAX_INTEGER INT_MAX

#elif SABER_INT_TYPE == SABER_INT_LONG

typedef long Integer;
#define SABER_MIN_INTEGER LONG_MIN
#define SABER_MAX_INTEGER LONG_MAX

#elif SABER_INT_TYPE == SABER_INT_LONGLONG

typedef long long Integer;
#define SABER_MIN_INTEGER LLONG_MIN
#define SABER_MAX_INTEGER LLONG_MAX

#endif

#if SABER_FLOAT_TYPE == SABER_FLOAT_FLOAT

typedef float Float;
#define SABER_INFINITY INFINITY

#elif SABER_FLOAT_TYPE == SABER_FLOAT_DOUBLE

typedef double Float;
#define SABER_INFINITY INFINITY

#endif

#define PI                  3.14159265358f
#define E                   2.718281828459f
#define SELF                "self"

inline float RadiansToDegree(float radians) {
	float degrees = radians * 180.0 / PI;
	return degrees;
}

inline float DegreeToRadians(float degrees) {
	float radians = degrees / 180.0 * PI;
	return radians;
}

#endif