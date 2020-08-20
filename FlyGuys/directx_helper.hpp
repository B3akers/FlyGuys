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
typedef unsigned long DWORD;

#pragma warning( push )
#pragma warning( disable : 4201)
typedef struct _D3DMATRIX {
	union {
		struct {
			float        _11, _12, _13, _14;
			float        _21, _22, _23, _24;
			float        _31, _32, _33, _34;
			float        _41, _42, _43, _44;

		};
		float m[ 4 ][ 4 ];
	};
} D3DMATRIX, D3DXMATRIX;
#pragma warning( pop ) 

typedef struct _D3DVECTOR {
	float x;
	float y;
	float z;
} D3DVECTOR, D3DXVECTOR3;

typedef struct _D3DVIEWPORT9 {
	DWORD       X;
	DWORD       Y;
	DWORD       Width;
	DWORD       Height;
	float       MinZ;
	float       MaxZ;
} D3DVIEWPORT9;

namespace d3d_helper {
	D3DXMATRIX* tmpD3DXMatrixMultiply( D3DXMATRIX* pout, const D3DXMATRIX* pm1, const D3DXMATRIX* pm2 );
	D3DXVECTOR3* tmpD3DXVec3TransformCoord( D3DXVECTOR3* pOut, const D3DXVECTOR3* pV, const D3DXMATRIX* pM );
	D3DXVECTOR3* tmpD3DXVec3Project( D3DXVECTOR3* pout, const D3DXVECTOR3* pv, const D3DVIEWPORT9* pviewport, const D3DXMATRIX* pprojection, const D3DXMATRIX* pview, const D3DXMATRIX* pworld );
	D3DXMATRIX* tmpD3DXMatrixIdentity( D3DXMATRIX* pOut );
};