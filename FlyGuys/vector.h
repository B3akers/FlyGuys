/* This file is part of FlyGuys by b3akers, licensed under the MIT license:
*
* MIT License
*
* Copyright (c) b3akers 2020
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*/
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