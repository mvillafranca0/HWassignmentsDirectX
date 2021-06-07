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

bool CollidedSkinModel(SKINMODEL *skinModel, D3DXMATRIX firstMatrix, MODEL *secondModel, D3DXMATRIX secondMatrix)
{
	ID3DXMesh *pMesh; // Secondary mesh container
	skinModel->mesh->CloneMeshFVF(0, skinModel->mesh->GetFVF(), d3ddev, &pMesh);

	// copy information from skin mesh to the secondary mesh and update it
	void *SrcPtr, *DestPtr;
	skinModel->mesh->LockVertexBuffer(D3DLOCK_READONLY,(void**)&SrcPtr);
	pMesh->LockVertexBuffer(0, (void**)&DestPtr);
	skinModel->skinInfo->UpdateSkinnedMesh(skinModel->skinMatrix, NULL, SrcPtr, DestPtr);
	skinModel->mesh->UnlockVertexBuffer();
	pMesh->UnlockVertexBuffer();
	
	// check the bounding box of the animated mesh
	MODEL *firstModel = (MODEL*) malloc (sizeof(MODEL));
	firstModel->mesh = pMesh;

	bool result;
	if (Collided(firstModel, firstMatrix, secondModel, secondMatrix) == true)
		result = true;
	else
		result = false;

	firstModel->mesh->Release();
	delete firstModel;

	return result;
}

bool CheckAnimationCollision(SKINMODEL *skinModel, D3DXMATRIX firstMatrix, float animationSpeed, MODEL *secondModel, D3DXMATRIX secondMatrix)
{
	bool result;
	LPD3DXANIMATIONCONTROLLER  anim;
	UINT n1, n2, n3, n4;

	// clone the original controller
	n1 = skinModel->animCtrl->GetMaxNumAnimationOutputs();
	n2 = skinModel->animCtrl->GetMaxNumAnimationSets();;
	n3 = skinModel->animCtrl->GetMaxNumTracks();
	n4 = skinModel->animCtrl->GetMaxNumEvents();
	skinModel->animCtrl->CloneAnimationController(n1, n2, n3, n4, &anim);

	// check if collided after animation
	UpdateSkinModel(skinModel, animationSpeed);
	if (CollidedSkinModel(skinModel, firstMatrix, secondModel, secondMatrix) == true)
		result = true;
	else 
		result = false;

	// restore the original animation controller
	skinModel->animCtrl->Release();
	anim->CloneAnimationController(n1, n2, n3, n4, &skinModel->animCtrl);
	anim->Release();
	UpdateSkinModel(skinModel, 0);

	return result;
}



void CalculateRay(int x, int y, D3DXVECTOR3 &rayOrigin, D3DXVECTOR3 &rayDirection)
{
    float px = 0.0f, py = 0.0f;

    // Get viewport
    D3DVIEWPORT9 vp;
    d3ddev->GetViewport(&vp);

    // Get Projection matrix
    D3DXMATRIX proj, view, viewInverse;
    d3ddev->GetTransform(D3DTS_PROJECTION, &proj);

    px = ((( 2.0f * x) / vp.Width)  - 1.0f) / proj(0, 0);
    py = (((-2.0f * y) / vp.Height) + 1.0f) / proj(1, 1);

    rayOrigin = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
    rayDirection = D3DXVECTOR3(px, py, 1.0f);

    d3ddev->GetTransform(D3DTS_VIEW, &view);
    D3DXMatrixInverse(&viewInverse, 0, &view);    
    D3DXVec3TransformCoord(&rayOrigin, &rayOrigin, 
	&viewInverse);
    D3DXVec3TransformNormal(&rayDirection, &rayDirection, 
	&viewInverse);
    D3DXVec3Normalize(&rayDirection, &rayDirection);
}

bool CollidedModelRay(MODEL * model, D3DXMATRIX transform, D3DXVECTOR3 rayOrigin,D3DXVECTOR3 rayDirection)
{
	void *pVertices = NULL;
	
	D3DXVECTOR3 minBounds, maxBounds;
	model->mesh->LockVertexBuffer( 
					D3DLOCK_READONLY, 
					(LPVOID*)&pVertices);
	D3DXComputeBoundingBox(
		(D3DXVECTOR3*)pVertices, 
		model->mesh->GetNumVertices(), 
		D3DXGetFVFVertexSize(model->mesh->GetFVF()), 
		&minBounds, &maxBounds);
	model->mesh->UnlockVertexBuffer();

	D3DXVECTOR3 objectBounds[8], worldBounds[8];
	objectBounds[0] = D3DXVECTOR3( minBounds.x, minBounds.y, minBounds.z); 
	objectBounds[1] = D3DXVECTOR3( maxBounds.x, minBounds.y, minBounds.z ); 
	objectBounds[2] = D3DXVECTOR3( minBounds.x, maxBounds.y, minBounds.z ); 
	objectBounds[3] = D3DXVECTOR3( maxBounds.x, maxBounds.y, minBounds.z ); 
	objectBounds[4] = D3DXVECTOR3( minBounds.x, minBounds.y, maxBounds.z ); 
	objectBounds[5] = D3DXVECTOR3( maxBounds.x, minBounds.y, maxBounds.z ); 
	objectBounds[6] = D3DXVECTOR3( minBounds.x, maxBounds.y, maxBounds.z ); 
	objectBounds[7] = D3DXVECTOR3( maxBounds.x, maxBounds.y, maxBounds.z ); 
	for( int i = 0; i < 8; i++ )
	{
      D3DXVec3TransformCoord( &worldBounds[i], &objectBounds[i], &transform );
	}
	D3DXVECTOR3 worldMinBounds = findMinBound(worldBounds, 8);
	D3DXVECTOR3 worldMaxBounds = findMaxBound(worldBounds, 8);

	return D3DXBoxBoundProbe(&worldMinBounds, &worldMaxBounds, &rayOrigin, &rayDirection);
}