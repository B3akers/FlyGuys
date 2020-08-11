#include "directx_helper.hpp"

D3DXMATRIX* d3d_helper::tmpD3DXMatrixMultiply( D3DXMATRIX* pout, const D3DXMATRIX* pm1, const D3DXMATRIX* pm2 ) {
	for ( int i = 0; i < 4; i++ ) {
		for ( int j = 0; j < 4; j++ ) {
			pout->m[ i ][ j ] = pm1->m[ i ][ 0 ] * pm2->m[ 0 ][ j ] + pm1->m[ i ][ 1 ] * pm2->m[ 1 ][ j ] + pm1->m[ i ][ 2 ] * pm2->m[ 2 ][ j ] + pm1->m[ i ][ 3 ] * pm2->m[ 3 ][ j ];
		}
	}

	return pout;
}

D3DXVECTOR3* d3d_helper::tmpD3DXVec3TransformCoord( D3DXVECTOR3* pOut, const D3DXVECTOR3* pV, const D3DXMATRIX* pM ) {
	float x, y, z, w;

	x = pV->x * pM->_11 + pV->y * pM->_21 + pV->z * pM->_31 + pM->_41;
	y = pV->x * pM->_12 + pV->y * pM->_22 + pV->z * pM->_32 + pM->_42;
	z = pV->x * pM->_13 + pV->y * pM->_23 + pV->z * pM->_33 + pM->_43;
	w = pV->x * pM->_14 + pV->y * pM->_24 + pV->z * pM->_34 + pM->_44;

	pOut->x = x / w;
	pOut->y = y / w;
	pOut->z = z / w;

	return pOut;
}

D3DXVECTOR3* d3d_helper::tmpD3DXVec3Project( D3DXVECTOR3* pout, const D3DXVECTOR3* pv, const D3DVIEWPORT9* pviewport, const D3DXMATRIX* pprojection, const D3DXMATRIX* pview, const D3DXMATRIX* pworld ) {
	D3DXMATRIX m1, m2;
	D3DXVECTOR3 vec;

	tmpD3DXMatrixMultiply( &m1, pworld, pview );
	tmpD3DXMatrixMultiply( &m2, &m1, pprojection );

	tmpD3DXVec3TransformCoord( &vec, pv, &m2 );
	pout->x = pviewport->X + ( 1.0f + vec.x ) * pviewport->Width / 2.0f;
	pout->y = pviewport->Y + ( 1.0f - vec.y ) * pviewport->Height / 2.0f;

	pout->z = pviewport->MinZ + vec.z * ( pviewport->MaxZ - pviewport->MinZ );

	return pout;
}

D3DXMATRIX* d3d_helper::tmpD3DXMatrixIdentity( D3DXMATRIX* pOut ) {
	pOut->m[ 0 ][ 1 ] = pOut->m[ 0 ][ 2 ] = pOut->m[ 0 ][ 3 ] =
		pOut->m[ 1 ][ 0 ] = pOut->m[ 1 ][ 2 ] = pOut->m[ 1 ][ 3 ] =
		pOut->m[ 2 ][ 0 ] = pOut->m[ 2 ][ 1 ] = pOut->m[ 2 ][ 3 ] =
		pOut->m[ 3 ][ 0 ] = pOut->m[ 3 ][ 1 ] = pOut->m[ 3 ][ 2 ] = 0.0f;

	pOut->m[ 0 ][ 0 ] = pOut->m[ 1 ][ 1 ] = pOut->m[ 2 ][ 2 ] = pOut->m[ 3 ][ 3 ] = 1.0f;
	return pOut;
}