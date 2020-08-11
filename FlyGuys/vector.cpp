#include "vector.h"
#include <math.h>

vector::vector( ) {
	x = 0;
	y = 0;
	z = 0;
}

vector::vector( float x, float y ) {
	this->x = x;
	this->y = y;
	this->z = 0;
}

vector::vector( float x, float y, float z ) {
	this->x = x;
	this->y = y;
	this->z = z;
}


bool vector::is_valid( ) {
	return this->x != 0 && this->y != 0;
}

bool vector::operator==( const vector& v_other ) const {
	return this->x == v_other.x && this->y == v_other.y && this->z == v_other.z;
}

bool vector::operator!=( const vector& v_other ) const {
	return this->x != v_other.x || this->y != v_other.y || this->z != v_other.z;
}

vector vector::operator*( const quaternion& quat ) {
	float num = quat.x * 2.f;
	float num2 = quat.y * 2.f;
	float num3 = quat.z * 2.f;
	float num4 = quat.x * num;
	float num5 = quat.y * num2 * -1.f;
	float num6 = quat.z * num3;
	float num7 = quat.x * num2;
	float num8 = quat.x * num3;
	float num9 = quat.y * num3;
	float num10 = quat.w * num;
	float num11 = quat.w * num2;
	float num12 = quat.w * num3 * -1.f;
	vector result;
	result.x = ( 1.f - ( num5 + num6 ) ) * x + ( num7 - num12 ) * y + ( num8 + num11 ) * z;
	result.y = ( num7 + num12 ) * x + ( 1.f - ( num4 + num6 ) ) * y + ( num9 - num10 ) * z;
	result.z = ( num8 - num11 ) * x + ( num9 + num10 ) * y + ( 1.f - ( num4 + num5 ) ) * z;
	return result;
}

vector& vector::operator=( const vector& vOther ) {
	x = vOther.x; y = vOther.y; z = vOther.z;
	return *this;
}

vector vector::operator-( ) const {
	return vector( -x, -y, -z );
}

vector vector::operator+( const vector& v ) const {
	return vector( this->x + v.x, this->y + v.y, this->z + v.z );
}

vector vector::operator-( const vector& v ) const {
	return vector( this->x - v.x, this->y - v.y, this->z - v.z );
}

vector vector::operator*( const vector& v ) const {
	return vector( this->x * v.x, this->y * v.y, this->z * v.z );
}

vector vector::operator/( const vector& v ) const {
	return vector( this->x / v.x, this->y / v.y, this->z / v.z );
}

vector vector::operator*( float fl ) const {
	return vector( this->x * fl, this->y * fl, this->z * fl );
}

vector vector::operator/( float fl ) const {
	return vector( this->x / fl, this->y / fl, this->z / fl );
}