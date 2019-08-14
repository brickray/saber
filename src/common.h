#ifndef H_COMMON_H
#define H_COMMON_H

#include <string>
#include <vector>
#include <stack>
#include <hash_set>
#include <hash_map>
#include <memory>

using namespace std;

#define PI                  3.14159265358f

inline float RadiansToDegree(float radians) {
	float degrees = radians * 180.0 / PI;
	return degrees;
}

inline float DegreeToRadians(float degrees) {
	float radians = degrees / 180.0 * PI;
	return radians;
}

#endif