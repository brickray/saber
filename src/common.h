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

#define PI                  3.14159265358f
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