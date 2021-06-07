#include "MyDirectX.h"

const int MAX_NUM_BONES_SUPPORTED = 35;

void BuildToRootXForms(FrameEx* frame,D3DXMATRIX& parentsToRoot)
{
	// Save some references to economize line space.
	D3DXMATRIX& toParent = frame->TransformationMatrix;
	D3DXMATRIX& toRoot = frame->toRoot;
	toRoot = toParent * parentsToRoot;
	FrameEx* sibling = (FrameEx*)frame->pFrameSibling;
	FrameEx* firstChild = (FrameEx*)frame->pFrameFirstChild;
	// Recurse down siblings.
	if( sibling )
		BuildToRootXForms(sibling, parentsToRoot);
	// Recurse to first child.
	if( firstChild )
		BuildToRootXForms(firstChild, toRoot);	
}

D3DXFRAME* FindNodeWithMesh(D3DXFRAME* frame)
{
	if (frame == NULL)
		return NULL;

	if( frame->pMeshContainer )
		if( frame->pMeshContainer->MeshData.pMesh != 0 )
			return frame;

	D3DXFRAME* f = 0;
	if(frame->pFrameSibling)
		if( f = FindNodeWithMesh(frame->pFrameSibling) )
			return f;

	if(frame->pFrameFirstChild)
		if( f = FindNodeWithMesh(frame->pFrameFirstChild) )
			return f;

	return NULL;
}

void BuildToRootXFormPtrArray(SKINMODEL* skinModel)
{
	skinModel->frameToRootMatrix = new D3DXMATRIX*[skinModel->numBones];
	skinModel->skinMatrix = new D3DXMATRIX[skinModel->numBones];
	
	for(long i = 0; i < skinModel->numBones; ++i)
	{
		// Find the frame that corresponds with the ith bone
		// offset matrix.
		const char* boneName = skinModel->skinInfo->GetBoneName(i);
		D3DXFRAME* frame = D3DXFrameFind(skinModel->root, boneName);
		if( frame )
		{
			FrameEx* frameEx = static_cast<FrameEx*>( frame );
			skinModel->frameToRootMatrix[i] = &(frameEx->toRoot);
		}
	}
}

SKINMODEL* LoadSkinModel(string filename)
{
	SKINMODEL *skinModel = (SKINMODEL*) malloc (sizeof(SKINMODEL));
	AllocMeshHierarchy allocMeshHierarchy;
	HRESULT result;

	result = D3DXLoadMeshHierarchyFromX(filename.c_str(), D3DXMESH_MANAGED, d3ddev, &allocMeshHierarchy, NULL, &skinModel->root, &skinModel->animCtrl);
	if (result != D3D_OK)
    {
        MessageBox(0, "Error loading model file", APPTITLE.c_str(), 0);
		free(skinModel);
        return NULL;
    }

	D3DXFRAME* f = FindNodeWithMesh(skinModel->root);
	if (f == NULL) 
	{
		free(skinModel);
		return NULL;
	}

	D3DXMESHCONTAINER *meshContainer = f->pMeshContainer;
	skinModel->skinInfo = meshContainer->pSkinInfo;			
	skinModel->numBones = skinModel->skinInfo->GetNumBones();
	skinModel->mesh = meshContainer->MeshData.pMesh;
	skinModel->material_count = meshContainer->NumMaterials;
	skinModel->materials = new D3DMATERIAL9[skinModel->material_count];
	skinModel->textures  = new LPDIRECT3DTEXTURE9[skinModel->material_count];

	//create the materials and textures
	for(DWORD i=0; i<skinModel->material_count; i++)
	{
		//grab the material
		skinModel->materials[i] = meshContainer->pMaterials->MatD3D;

	    //set ambient color for material 
		skinModel->materials[i].Ambient = skinModel->materials[i].Diffuse;

	    skinModel->textures[i] = NULL;
	    if (meshContainer->pMaterials[i].pTextureFilename != NULL) 
		{
	        string filename = meshContainer->pMaterials[i].pTextureFilename;
			ifstream file;
			file.open(filename);
			if (file) 
			{
				result = D3DXCreateTextureFromFile(d3ddev, filename.c_str(), &skinModel->textures[i]);
				if (result != D3D_OK) 
				{
				    MessageBox(NULL, filename.c_str(), "Error", MB_OK);
					return NULL;
			    }
				file.close();
			}
		}
	}
	BuildToRootXFormPtrArray(skinModel);
	UpdateSkinModel(skinModel, 0);

	return skinModel;
}

void UpdateSkinModel(SKINMODEL* skinModel, float speed)
{
	// The AnimationController updates bone matrices to reflect
	// the given pose at the current time by interpolating between
	// animation keyframes.
	skinModel->animCtrl->AdvanceTime(speed, 0);

	// Recurse down the tree and generate a frame's toRoot
	// transform from the updated pose.
	D3DXMATRIX identity;
	D3DXMatrixIdentity(&identity);
	BuildToRootXForms((FrameEx*)skinModel->root, identity);

	// Build the final transforms for each bone (Equation 16.2)
	D3DXMATRIX offsetTemp, toRootTemp;
	for(long i = 0; i < skinModel->numBones; ++i)
	{
		offsetTemp = *skinModel->skinInfo->GetBoneOffsetMatrix(i);
		toRootTemp = *skinModel->frameToRootMatrix[i];
		skinModel->skinMatrix[i] = offsetTemp * toRootTemp;
	}
}

void DrawSkinModel(SKINMODEL* skinModel)
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
	
	for( DWORD i=0; i < skinModel->material_count; i++ )
	{
		d3ddev->SetMaterial (&skinModel->materials[i]);
		d3ddev->SetTexture (0, skinModel->textures[i]);
		pMesh->DrawSubset(i);
	}
	pMesh->Release();
}

void DeleteSkinModel(SKINMODEL* skinModel)
{
	AllocMeshHierarchy allocMeshHierarchy;
	if (skinModel->root)
		D3DXFrameDestroy(skinModel->root, &allocMeshHierarchy);

	if (skinModel->skinMatrix)
		delete[] skinModel->skinMatrix;

	if (skinModel->frameToRootMatrix)
		delete[] skinModel->frameToRootMatrix;

    //remove materials from memory
    if ( skinModel->materials != NULL ) 
        delete[] skinModel->materials;

    //remove textures from memory
    if (skinModel->textures != NULL)
    {
        for( DWORD i = 0; i < skinModel->material_count; i++)
        {
            if (skinModel->textures[i] != NULL)
                skinModel->textures[i]->Release();
        }
        delete[] skinModel->textures;
    }

	if (skinModel->animCtrl)
		skinModel->animCtrl->Release();

    //remove model struct from memory
    if (skinModel != NULL)
        free(skinModel);
}
