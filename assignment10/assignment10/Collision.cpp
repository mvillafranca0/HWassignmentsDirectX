#include "MyDirectX.h"

D3DXVECTOR3 findMinBound(D3DXVECTOR3* vectors, int arraySize)
{
	float x, y, z;
	x = vectors[0].x; 
	y = vectors[0].y; 
	z = vectors[0].z; 
	for (int i=0; i<arraySize; i++)
	{
		if (x > vectors[i].x) 
			x = vectors[i].x;
		if (y > vectors[i].y) 
			y = vectors[i].y;
		if (z > vectors[i].z) 
			z = vectors[i].z;
	}
	return D3DXVECTOR3(x, y, z);
}

D3DXVECTOR3 findMaxBound(D3DXVECTOR3* vectors, int arraySize)
{
	float x, y, z;
	x = vectors[0].x; 
	y = vectors[0].y; 
	z = vectors[0].z; 
	for (int i=0; i<arraySize; i++)
	{
		if (x < vectors[i].x) 
			x = vectors[i].x;
		if (y < vectors[i].y) 
			y = vectors[i].y;
		if (z < vectors[i].z) 
			z = vectors[i].z;
	}
	return D3DXVECTOR3(x, y, z);
}

bool Collided(MODEL * firstModel, D3DXMATRIX firstMatrix, MODEL * secondModel, D3DXMATRIX secondMatrix)
{
	void *pVertices = NULL;
	
	D3DXVECTOR3 firstMinBounds, firstMaxBounds;
	firstModel->mesh->LockVertexBuffer( 
					D3DLOCK_READONLY, 
					(LPVOID*)&pVertices);
	D3DXComputeBoundingBox(
		(D3DXVECTOR3*)pVertices, 
		firstModel->mesh->GetNumVertices(), 
		D3DXGetFVFVertexSize(firstModel->mesh->GetFVF()), 
		&firstMinBounds, &firstMaxBounds);
	firstModel->mesh->UnlockVertexBuffer();

	D3DXVECTOR3 secondMinBounds, secondMaxBounds;
	secondModel->mesh->LockVertexBuffer( 
			D3DLOCK_READONLY, 
			(LPVOID*)&pVertices);
	D3DXComputeBoundingBox(
		(D3DXVECTOR3*)pVertices, 
		secondModel->mesh->GetNumVertices(), 	
		D3DXGetFVFVertexSize(secondModel->mesh->GetFVF()), 
		&secondMinBounds, &secondMaxBounds);
	secondModel->mesh->UnlockVertexBuffer();

	D3DXVECTOR3 first_objectBounds[8], first_worldBounds[8];
	first_objectBounds[0] = D3DXVECTOR3( firstMinBounds.x, firstMinBounds.y, firstMinBounds.z); 
	first_objectBounds[1] = D3DXVECTOR3( firstMaxBounds.x, firstMinBounds.y, firstMinBounds.z ); 
	first_objectBounds[2] = D3DXVECTOR3( firstMinBounds.x, firstMaxBounds.y, firstMinBounds.z ); 
	first_objectBounds[3] = D3DXVECTOR3( firstMaxBounds.x, firstMaxBounds.y, firstMinBounds.z ); 
	first_objectBounds[4] = D3DXVECTOR3( firstMinBounds.x, firstMinBounds.y, firstMaxBounds.z ); 
	first_objectBounds[5] = D3DXVECTOR3( firstMaxBounds.x, firstMinBounds.y, firstMaxBounds.z ); 
	first_objectBounds[6] = D3DXVECTOR3( firstMinBounds.x, firstMaxBounds.y, firstMaxBounds.z ); 
	first_objectBounds[7] = D3DXVECTOR3( firstMaxBounds.x, firstMaxBounds.y, firstMaxBounds.z ); 
	for( int i = 0; i < 8; i++ )
	{
      D3DXVec3TransformCoord( &first_worldBounds[i], &first_objectBounds[i], &firstMatrix );
	}
	D3DXVECTOR3 firstWorldMinBounds = findMinBound(first_worldBounds, 8);
	D3DXVECTOR3 firstWorldMaxBounds = findMaxBound(first_worldBounds, 8);

	D3DXVECTOR3 second_objectBounds[8], second_worldBounds[8];
	second_objectBounds[0] = D3DXVECTOR3( secondMinBounds.x, secondMinBounds.y, secondMinBounds.z); 
	second_objectBounds[1] = D3DXVECTOR3( secondMaxBounds.x, secondMinBounds.y, secondMinBounds.z ); 
	second_objectBounds[2] = D3DXVECTOR3( secondMinBounds.x, secondMaxBounds.y, secondMinBounds.z ); 
	second_objectBounds[3] = D3DXVECTOR3( secondMaxBounds.x, secondMaxBounds.y, secondMinBounds.z ); 
	second_objectBounds[4] = D3DXVECTOR3( secondMinBounds.x, secondMinBounds.y, secondMaxBounds.z ); 
	second_objectBounds[5] = D3DXVECTOR3( secondMaxBounds.x, secondMinBounds.y, secondMaxBounds.z ); 
	second_objectBounds[6] = D3DXVECTOR3( secondMinBounds.x, secondMaxBounds.y, secondMaxBounds.z ); 
	second_objectBounds[7] = D3DXVECTOR3( secondMaxBounds.x, secondMaxBounds.y, secondMaxBounds.z ); 
	for( int i = 0; i < 8; i++ )
	{
      D3DXVec3TransformCoord( &second_worldBounds[i], &second_objectBounds[i], &secondMatrix );
	}
	D3DXVECTOR3 secondWorldMinBounds = findMinBound(second_worldBounds, 8);
	D3DXVECTOR3 secondWorldMaxBounds = findMaxBound(second_worldBounds, 8);

	// if the max x position of A is less than the min x position of B they do not collide
	// if the min x position of A is greater than the max x position of B they do not collide
	// and the same goes for y and z
	if (firstWorldMinBounds.x > secondWorldMaxBounds.x) 
		return false;
	if (firstWorldMaxBounds.x < secondWorldMinBounds.x)
		return false;
	if (firstWorldMinBounds.y > secondWorldMaxBounds.y) 
		return false;
	if (firstWorldMaxBounds.y < secondWorldMinBounds.y)
		return false;
	if (firstWorldMinBounds.z > secondWorldMaxBounds.z) 
		return false;
	if (firstWorldMaxBounds.z < secondWorldMinBounds.z)
		return false;

	return true;
}
