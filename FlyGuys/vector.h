#pragma once
#include <inttypes.h>
#include <cmath>
#include <cfloat>

#define M_PI 3.14159265358979323846

class quaternion {
public:
	float x;
	float z;
	float y;
	float w;
};

class vector
{
public:
	float x;
	float z;
	float y;

	vector();
	vector(float x, float y);
	vector(float x, float y, float z);

	vector& operator=(const vector& vOther);

	vector operator-() const;
	vector operator+(const vector& v) const;
	vector operator-(const vector& v) const;
	vector operator*(const vector& v) const;
	vector operator/(const vector& v) const;
	vector operator*(float fl) const;
	vector operator/(float fl) const;
	vector operator*( const quaternion& quat );

	bool is_valid();

	bool operator==(const vector& v_other) const;
	bool operator!=(const vector& v_other) const;
};